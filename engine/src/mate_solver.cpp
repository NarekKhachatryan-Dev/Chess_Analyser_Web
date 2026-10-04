#include "chess_engine.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <unordered_map>
#include <utility>

namespace chesslab {
namespace {

struct SearchResult {
    bool found = false;
    int attacker_moves = 0;
    std::vector<Move> line;
};

struct TableEntry {
    SearchResult result;
};

class Search {
public:
    Search(Position& root, Color attacker, int mate_in, const SolveOptions& options)
        : root_(root), attacker_(attacker), max_plies_(2 * mate_in - 1), options_(options), start_(std::chrono::steady_clock::now()) {}

    SolveResult run() {
        SolveResult result;
        const auto legal_moves = root_.generate_legal_moves();
        if (legal_moves.empty()) {
            result.status = SolveStatus::NoLegalMoves;
            result.nodes = 1;
            result.elapsed_ms = elapsed_ms();
            return result;
        }

        const SearchResult search_result = visit(root_, max_plies_, 0);
        result.nodes = nodes_;
        result.elapsed_ms = elapsed_ms();
        if (cancelled_) {
            result.status = timed_out_ ? SolveStatus::TimedOut : SolveStatus::Cancelled;
        } else if (!search_result.found) {
            result.status = SolveStatus::NoMateWithinN;
        } else {
            result.status = SolveStatus::MateFound;
            result.length = search_result.attacker_moves;
            result.line = search_result.line;
        }
        return result;
    }

private:
    std::vector<Move> ordered_moves(const Position& position, const std::vector<Move>& moves, int ply) {
        if (!options_.use_move_ordering) {
            return moves;
        }
        struct ScoredMove {
            Move move;
            int score;
        };
        std::vector<ScoredMove> scored;
        scored.reserve(moves.size());
        for (const Move& move : moves) {
            int score = move.is_capture ? 100 : 0;
            Position child = position;
            child.make_move(move);
            if (child.is_in_check(opposite(position.side_to_move))) {
                score += 1000;
            }
            if (ply < static_cast<int>(killers_.size())) {
                for (int slot = 0; slot < 2; ++slot) {
                    if (same_move(move, killers_[ply][slot])) {
                        score += 200 - slot;
                    }
                }
            }
            scored.push_back({move, score});
        }
        std::stable_sort(scored.begin(), scored.end(), [](const ScoredMove& left, const ScoredMove& right) {
            return left.score > right.score;
        });
        std::vector<Move> result;
        result.reserve(scored.size());
        for (const ScoredMove& item : scored) {
            result.push_back(item.move);
        }
        return result;
    }

    static bool same_move(const Move& left, const Move& right) {
        return left.from == right.from && left.to == right.to && left.promotion == right.promotion;
    }

    void remember_killer(const Move& move, int ply) {
        if (ply >= static_cast<int>(killers_.size()) || move.is_capture) {
            return;
        }
        if (!same_move(move, killers_[ply][0])) {
            killers_[ply][1] = killers_[ply][0];
            killers_[ply][0] = move;
        }
    }

    SearchResult visit(Position& position, int plies_remaining, int ply) {
        ++nodes_;
        if (stop_requested()) {
            cancelled_ = true;
            return {};
        }
        const uint64_t key = position_key(position, plies_remaining);
        if (options_.use_transposition_table) {
            const auto found = table_.find(key);
            if (found != table_.end()) {
                return found->second.result;
            }
        }

        const auto legal_moves = position.generate_legal_moves();
        if (legal_moves.empty()) {
            SearchResult result{position.is_in_check(position.side_to_move) && position.side_to_move != attacker_, 0, {}};
            store(key, result);
            return result;
        }
        if (plies_remaining == 0) {
            SearchResult result;
            store(key, result);
            return result;
        }

        const auto moves = ordered_moves(position, legal_moves, ply);
        if (position.side_to_move == attacker_) {
            for (const auto& move : moves) {
                Position child = position;
                child.make_move(move);
                SearchResult child_result = visit(child, plies_remaining - 1, ply + 1);
                if (cancelled_) {
                    return {};
                }
                if (child_result.found) {
                    child_result.attacker_moves += 1;
                    child_result.line.insert(child_result.line.begin(), move);
                    store(key, child_result);
                    return child_result;
                }
            }
            SearchResult result;
            store(key, result);
            return result;
        }

        SearchResult worst_defence;
        worst_defence.found = true;
        worst_defence.attacker_moves = std::numeric_limits<int>::min();
        for (const auto& move : moves) {
            Position child = position;
            child.make_move(move);
            SearchResult child_result = visit(child, plies_remaining - 1, ply + 1);
            if (cancelled_ || !child_result.found) {
                if (!cancelled_) {
                    remember_killer(move, ply);
                    SearchResult result;
                    store(key, result);
                }
                return {};
            }
            if (child_result.attacker_moves > worst_defence.attacker_moves) {
                worst_defence.attacker_moves = child_result.attacker_moves;
                worst_defence.line = child_result.line;
                worst_defence.line.insert(worst_defence.line.begin(), move);
            }
        }
        store(key, worst_defence);
        return worst_defence;
    }

    static uint64_t mix(uint64_t value) {
        value += 0x9e3779b97f4a7c15ULL;
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31);
    }

    uint64_t position_key(const Position& position, int plies_remaining) const {
        uint64_t key = mix(static_cast<uint64_t>(plies_remaining));
        for (int square = 0; square < 64; ++square) {
            key ^= mix(static_cast<uint64_t>(static_cast<int>(position.board[square]) + 1) * 67ULL +
                       static_cast<uint64_t>(square) * 131ULL);
        }
        key ^= mix(static_cast<uint64_t>(position.side_to_move == Color::White ? 1 : 2));
        key ^= mix(static_cast<uint64_t>(position.castling_rights) + 17ULL);
        key ^= mix(static_cast<uint64_t>(position.en_passant_square + 2));
        return key;
    }

    void store(uint64_t key, const SearchResult& result) {
        if (!options_.use_transposition_table || table_.size() >= table_capacity()) {
            return;
        }
        table_.insert_or_assign(key, TableEntry{result});
    }

    size_t table_capacity() const {
        const uint64_t bytes = options_.transposition_table_size_mb * 1024ULL * 1024ULL;
        return static_cast<size_t>(std::max<uint64_t>(1, bytes / sizeof(std::pair<const uint64_t, TableEntry>)));
    }

    bool stop_requested() {
        if (options_.cancel != nullptr && options_.cancel->load()) {
            return true;
        }
        if (options_.time_limit_ms != 0 && elapsed_ms() >= options_.time_limit_ms) {
            timed_out_ = true;
            return true;
        }
        return false;
    }

    uint64_t elapsed_ms() const {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_).count());
    }

    Position& root_;
    Color attacker_;
    int max_plies_;
    SolveOptions options_;
    std::chrono::steady_clock::time_point start_;
    uint64_t nodes_ = 0;
    bool cancelled_ = false;
    bool timed_out_ = false;
    std::vector<std::array<Move, 2>> killers_ = std::vector<std::array<Move, 2>>(128);
    std::unordered_map<uint64_t, TableEntry> table_;
};

}  // namespace

std::string move_to_uci(const Move& move) {
    std::string result;
    result += static_cast<char>('a' + file_of(move.from));
    result += static_cast<char>('1' + rank_of(move.from));
    result += static_cast<char>('a' + file_of(move.to));
    result += static_cast<char>('1' + rank_of(move.to));
    switch (move.promotion) {
        case PieceType::Queen:
            result += 'q';
            break;
        case PieceType::Rook:
            result += 'r';
            break;
        case PieceType::Bishop:
            result += 'b';
            break;
        case PieceType::Knight:
            result += 'n';
            break;
        default:
            break;
    }
    return result;
}

SolveResult solve_depth(Position& position, int mate_in, const SolveOptions& options) {
    if (mate_in < 1) {
        SolveResult result;
        result.status = SolveStatus::NoMateWithinN;
        return result;
    }
    Search search(position, position.side_to_move, mate_in, options);
    return search.run();
}

SolveResult solve(Position& position, int max_mate_in, const SolveOptions& options) {
    if (max_mate_in < 1) {
        SolveResult result;
        result.status = SolveStatus::NoMateWithinN;
        return result;
    }

    SolveResult aggregate;
    for (int mate_in = 1; mate_in <= max_mate_in; ++mate_in) {
        SolveResult current = solve_depth(position, mate_in, options);
        aggregate.nodes += current.nodes;
        aggregate.elapsed_ms += current.elapsed_ms;
        if (current.status != SolveStatus::NoMateWithinN) {
            current.nodes = aggregate.nodes;
            current.elapsed_ms = aggregate.elapsed_ms;
            return current;
        }
    }
    aggregate.status = SolveStatus::NoMateWithinN;
    return aggregate;
}

}  // namespace chesslab
