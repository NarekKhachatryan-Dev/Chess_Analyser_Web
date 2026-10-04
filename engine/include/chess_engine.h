#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace chesslab {

inline constexpr const char* kProjectName = "ChessSolver";
inline constexpr int kEngineVersion = 1;

int engine_version();

enum class Color : uint8_t { White, Black };
enum class PieceType : uint8_t { Pawn, Knight, Bishop, Rook, Queen, King, None };
enum class Piece : uint8_t {
    Empty = 0,
    WhitePawn = 1,
    WhiteKnight = 2,
    WhiteBishop = 3,
    WhiteRook = 4,
    WhiteQueen = 5,
    WhiteKing = 6,
    BlackPawn = 7,
    BlackKnight = 8,
    BlackBishop = 9,
    BlackRook = 10,
    BlackQueen = 11,
    BlackKing = 12
};

enum CastlingRights : uint8_t {
    NoCastling = 0,
    WhiteKingSide = 1 << 0,
    WhiteQueenSide = 1 << 1,
    BlackKingSide = 1 << 2,
    BlackQueenSide = 1 << 3
};

inline constexpr Piece piece_from(Color color, PieceType type) {
    if (type == PieceType::None) {
        return Piece::Empty;
    }
    const int value = static_cast<int>(type) + 1;
    return color == Color::White ? static_cast<Piece>(value) : static_cast<Piece>(value + 6);
}

inline bool is_white_piece(Piece piece) {
    return piece >= Piece::WhitePawn && piece <= Piece::WhiteKing;
}

inline bool is_black_piece(Piece piece) {
    return piece >= Piece::BlackPawn && piece <= Piece::BlackKing;
}

inline Color color_of(Piece piece) {
    if (piece == Piece::Empty) {
        return Color::White;
    }
    return is_white_piece(piece) ? Color::White : Color::Black;
}

inline PieceType piece_type(Piece piece) {
    if (piece == Piece::Empty) {
        return PieceType::None;
    }
    const int value = static_cast<int>(piece);
    if (value >= static_cast<int>(Piece::WhitePawn) && value <= static_cast<int>(Piece::WhiteKing)) {
        return static_cast<PieceType>(value - 1);
    }
    return static_cast<PieceType>(value - 7);
}

inline constexpr Color opposite(Color side) {
    return side == Color::White ? Color::Black : Color::White;
}

inline constexpr int to_index(int file, int rank) {
    return rank * 8 + file;
}

inline constexpr int file_of(int square) {
    return square % 8;
}

inline constexpr int rank_of(int square) {
    return square / 8;
}

struct Move {
    int from = -1;
    int to = -1;
    PieceType promotion = PieceType::None;
    bool is_capture = false;
    bool is_en_passant = false;
    bool is_castle = false;
    bool is_double_pawn_push = false;
    bool is_king_side_castle = false;
    bool is_queen_side_castle = false;
};

struct ValidationResult {
    bool valid = true;
    std::vector<std::string> errors;
};

enum class SolveStatus : uint8_t { MateFound, NoMateWithinN, NoLegalMoves, Cancelled, TimedOut };

struct SolveOptions {
    const std::atomic_bool* cancel = nullptr;
    uint64_t time_limit_ms = 0;
    bool use_move_ordering = false;
    bool use_transposition_table = false;
    uint64_t transposition_table_size_mb = 64;
};

struct SolveResult {
    SolveStatus status = SolveStatus::NoMateWithinN;
    int length = 0;
    uint64_t nodes = 0;
    uint64_t elapsed_ms = 0;
    std::vector<Move> line;
};

std::string move_to_uci(const Move& move);

class Position {
public:
    struct UndoState {
        Piece moved_piece = Piece::Empty;
        Piece captured_piece = Piece::Empty;
        int from = -1;
        int to = -1;
        int captured_square = -1;
        int en_passant_square = -1;
        uint8_t castling_rights = 0;
        int halfmove_clock = 0;
        int fullmove_count = 1;
        bool was_en_passant = false;
    };

    std::array<Piece, 64> board{};
    Color side_to_move = Color::White;
    int en_passant_square = -1;
    uint8_t castling_rights = static_cast<uint8_t>(CastlingRights::WhiteKingSide | CastlingRights::WhiteQueenSide |
                                                  CastlingRights::BlackKingSide | CastlingRights::BlackQueenSide);
    int halfmove_clock = 0;
    int fullmove_count = 1;

    Position();
    static Position from_fen(const std::string& fen);
    std::string to_fen() const;

    void make_move(const Move& move);
    void unmake_move();
    std::vector<Move> generate_legal_moves() const;
    bool is_in_check(Color side) const;

private:
    std::vector<UndoState> history;
};

bool is_square_attacked(const Position& position, int square, Color by_color);
bool is_checkmate(const Position& position);
bool is_stalemate(const Position& position);
ValidationResult validate_position(const Position& position);
SolveResult solve_depth(Position& position, int mate_in, const SolveOptions& options = {});
SolveResult solve(Position& position, int max_mate_in, const SolveOptions& options = {});
uint64_t perft(Position& position, int depth);

}  // namespace chesslab
