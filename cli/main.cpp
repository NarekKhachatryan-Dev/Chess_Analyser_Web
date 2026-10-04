#include "chess_engine.h"

#include <iostream>
#include <string>
#include <chrono>
#include <array>
#include <utility>

namespace {

uint64_t count_nodes(chesslab::Position& position, int depth) {
    if (depth == 0) {
        return 1;
    }

    const auto moves = position.generate_legal_moves();
    if (depth == 1) {
        return static_cast<uint64_t>(moves.size());
    }

    uint64_t nodes = 0;
    for (const auto& move : moves) {
        chesslab::Position child = position;
        child.make_move(move);
        nodes += count_nodes(child, depth - 1);
    }
    return nodes;
}

char promotion_char(chesslab::PieceType promotion) {
    switch (promotion) {
        case chesslab::PieceType::Queen:
            return 'q';
        case chesslab::PieceType::Rook:
            return 'r';
        case chesslab::PieceType::Bishop:
            return 'b';
        case chesslab::PieceType::Knight:
            return 'n';
        default:
            return '\0';
    }
}

std::string move_to_coordinate(const chesslab::Move& move) {
    std::string result;
    result += static_cast<char>('a' + chesslab::file_of(move.from));
    result += static_cast<char>('1' + chesslab::rank_of(move.from));
    result += static_cast<char>('a' + chesslab::file_of(move.to));
    result += static_cast<char>('1' + chesslab::rank_of(move.to));
    if (const char promotion = promotion_char(move.promotion); promotion != '\0') {
        result += promotion;
    }
    return result;
}

int run_divide(const std::string& fen, int depth) {
    if (depth < 1) {
        std::cerr << "divide depth must be at least 1\n";
        return 1;
    }

    chesslab::Position position;
    try {
        position = chesslab::Position::from_fen(fen);
    } catch (const std::exception& error) {
        std::cerr << "invalid FEN: " << error.what() << '\n';
        return 1;
    }

    for (const auto& move : position.generate_legal_moves()) {
        chesslab::Position child = position;
        child.make_move(move);
        std::cout << move_to_coordinate(move) << ": " << count_nodes(child, depth - 1) << '\n';
    }
    return 0;
}

const char* status_name(chesslab::SolveStatus status) {
    switch (status) {
        case chesslab::SolveStatus::MateFound:
            return "MateFound";
        case chesslab::SolveStatus::NoMateWithinN:
            return "NoMateWithinN";
        case chesslab::SolveStatus::NoLegalMoves:
            return "NoLegalMoves";
        case chesslab::SolveStatus::Cancelled:
            return "Cancelled";
        case chesslab::SolveStatus::TimedOut:
            return "TimedOut";
    }
    return "Unknown";
}

int run_solve(const std::string& fen, int max_mate_in) {
    if (max_mate_in < 1) {
        std::cerr << "solve N must be at least 1\n";
        return 1;
    }

    try {
        auto position = chesslab::Position::from_fen(fen);
        chesslab::SolveOptions options;
        options.use_move_ordering = true;
        options.use_transposition_table = true;
        const auto result = chesslab::solve(position, max_mate_in, options);
        std::cout << status_name(result.status);
        if (result.status == chesslab::SolveStatus::MateFound) {
            std::cout << " length=" << result.length << " line=";
            for (size_t index = 0; index < result.line.size(); ++index) {
                if (index != 0) {
                    std::cout << ' ';
                }
                std::cout << chesslab::move_to_uci(result.line[index]);
            }
        }
        std::cout << " nodes=" << result.nodes << " time_ms=" << result.elapsed_ms << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "invalid FEN: " << error.what() << '\n';
        return 1;
    }
}

int run_bench() {
    const std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    auto position = chesslab::Position::from_fen(start_fen);
    const auto start = std::chrono::steady_clock::now();
    const auto nodes = chesslab::perft(position, 5);
    const auto stop = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "perft_start_d5 nodes=" << nodes << " time_ms=" << elapsed << '\n';

    const std::array<std::pair<const char*, const char*>, 3> hard_positions = {{
        {"qeP68", "r1b2kr1/ppppn3/5p1p/3P3P/4P1q1/2Q5/PB4P1/nN3RK1 w - - 0 21"},
        {"46thE", "2rqk2r/pp1nb1p1/4pnp1/3p4/3P3B/2P2N1P/PPQ2PP1/R3K2R w KQk - 0 17"},
        {"8dZpc", "5r1k/qR4p1/P6p/8/4Q3/2BP2PP/6K1/8 b - - 2 45"},
    }};
    for (const auto& [id, fen] : hard_positions) {
        auto no_table_position = chesslab::Position::from_fen(fen);
        chesslab::SolveOptions no_table_options;
        no_table_options.use_move_ordering = true;
        const auto no_table = chesslab::solve(no_table_position, 4, no_table_options);

        auto table_position = chesslab::Position::from_fen(fen);
        chesslab::SolveOptions table_options;
        table_options.use_move_ordering = true;
        table_options.use_transposition_table = true;
        const auto table = chesslab::solve(table_position, 4, table_options);
        std::cout << "solver_" << id << " no_tt_nodes=" << no_table.nodes
                  << " no_tt_time_ms=" << no_table.elapsed_ms << " tt_nodes=" << table.nodes
                  << " tt_time_ms=" << table.elapsed_ms << '\n';
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc >= 2 && std::string(argv[1]) == "divide") {
        if (argc != 4) {
            std::cerr << "usage: chess_cli divide \"<fen>\" <depth>\n";
            return 1;
        }

        try {
            const int depth = std::stoi(argv[3]);
            return run_divide(argv[2], depth);
        } catch (const std::exception& error) {
            std::cerr << "invalid depth: " << error.what() << '\n';
            return 1;
        }
    }

    if (argc >= 2 && std::string(argv[1]) == "solve") {
        if (argc != 4) {
            std::cerr << "usage: chess_cli solve \"<fen>\" <n>\n";
            return 1;
        }
        try {
            return run_solve(argv[2], std::stoi(argv[3]));
        } catch (const std::exception& error) {
            std::cerr << "invalid N: " << error.what() << '\n';
            return 1;
        }
    }

    if (argc == 2 && std::string(argv[1]) == "bench") {
        return run_bench();
    }

    std::cout << chesslab::kProjectName << " CLI\n";
    return argc == 1 ? 0 : 1;
}
