#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace chesslab {

inline constexpr const char* kProjectName = "ChessLab";
inline constexpr int kEngineVersion = 1;

int engine_version();

enum class Color : uint8_t { White, Black };
enum class PieceType : uint8_t { Pawn, Knight, Bishop, Rook, Queen, King, None };
enum class Piece : uint8_t {
    Empty = 0,
    WhitePawn,
    WhiteKnight,
    WhiteBishop,
    WhiteRook,
    WhiteQueen,
    WhiteKing,
    BlackPawn,
    BlackKnight,
    BlackBishop,
    BlackRook,
    BlackQueen,
    BlackKing
};

enum CastlingRights : uint8_t {
    NoCastling = 0,
    WhiteKingSide = 1 << 0,
    WhiteQueenSide = 1 << 1,
    BlackKingSide = 1 << 2,
    BlackQueenSide = 1 << 3
};

constexpr Piece piece_from(Color color, PieceType type) {
    if (type == PieceType::None) {
        return Piece::Empty;
    }
    const int index = static_cast<int>(type) + 1;
    if (color == Color::White) {
        return static_cast<Piece>(index);
    }
    return static_cast<Piece>(index + 6);
}

inline bool is_white_piece(Piece piece) {
    return piece != Piece::Empty && static_cast<int>(piece) <= static_cast<int>(Piece::WhiteKing);
}

inline bool is_black_piece(Piece piece) {
    return piece != Piece::Empty && static_cast<int>(piece) >= static_cast<int>(Piece::BlackPawn);
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

inline Color opposite(Color side) {
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

    static Move normal(int from_square, int to_square) {
        Move move;
        move.from = from_square;
        move.to = to_square;
        return move;
    }

    static Move capture(int from_square, int to_square) {
        Move move = normal(from_square, to_square);
        move.is_capture = true;
        return move;
    }
};

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
        bool was_castle = false;
        bool was_en_passant = false;
        int rook_from = -1;
        int rook_to = -1;
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
uint64_t perft(Position& position, int depth);

}  // namespace chesslab
