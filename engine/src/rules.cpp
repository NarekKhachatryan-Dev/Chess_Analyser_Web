#include "chess_engine.h"

#include <string>

namespace chesslab {

bool is_checkmate(const Position& position) {
    return position.is_in_check(position.side_to_move) && position.generate_legal_moves().empty();
}

bool is_stalemate(const Position& position) {
    return !position.is_in_check(position.side_to_move) && position.generate_legal_moves().empty();
}

ValidationResult validate_position(const Position& position) {
    ValidationResult result;
    int white_kings = 0;
    int black_kings = 0;
    int white_king_square = -1;
    int black_king_square = -1;

    for (int square = 0; square < 64; ++square) {
        const Piece piece = position.board[square];
        if (piece == piece_from(Color::White, PieceType::King)) {
            ++white_kings;
            white_king_square = square;
        } else if (piece == piece_from(Color::Black, PieceType::King)) {
            ++black_kings;
            black_king_square = square;
        } else if (piece_type(piece) == PieceType::Pawn && (rank_of(square) == 0 || rank_of(square) == 7)) {
            result.errors.emplace_back("Pawns cannot be placed on the first or eighth rank.");
        }
    }

    if (white_kings != 1) {
        result.errors.emplace_back("Position must contain exactly one white king.");
    }
    if (black_kings != 1) {
        result.errors.emplace_back("Position must contain exactly one black king.");
    }

    if (white_king_square >= 0 && black_king_square >= 0) {
        const int file_distance = file_of(white_king_square) - file_of(black_king_square);
        const int rank_distance = rank_of(white_king_square) - rank_of(black_king_square);
        if (file_distance >= -1 && file_distance <= 1 && rank_distance >= -1 && rank_distance <= 1) {
            result.errors.emplace_back("Kings cannot be adjacent.");
        }
    }

    if (black_kings == 1 && position.is_in_check(Color::Black) && position.side_to_move == Color::White) {
        result.errors.emplace_back("The side not to move cannot be in check.");
    }
    if (white_kings == 1 && position.is_in_check(Color::White) && position.side_to_move == Color::Black) {
        result.errors.emplace_back("The side not to move cannot be in check.");
    }

    if (position.en_passant_square >= 0) {
        const int square = position.en_passant_square;
        const int expected_rank = position.side_to_move == Color::White ? 5 : 2;
        const int pawn_square = position.side_to_move == Color::White ? square - 8 : square + 8;
        const Piece expected_pawn = piece_from(opposite(position.side_to_move), PieceType::Pawn);

        if (rank_of(square) != expected_rank) {
            result.errors.emplace_back("En passant square is on the wrong rank for the side to move.");
        } else if (position.board[square] != Piece::Empty) {
            result.errors.emplace_back("En passant square must be empty.");
        } else if (pawn_square < 0 || pawn_square >= 64 || position.board[pawn_square] != expected_pawn) {
            result.errors.emplace_back("En passant square must have the opponent pawn directly behind it.");
        }
    }

    const auto requires_piece = [&](uint8_t right, int square, Piece expected, const char* message) {
        if ((position.castling_rights & right) != 0 && position.board[square] != expected) {
            result.errors.emplace_back(message);
        }
    };
    requires_piece(static_cast<uint8_t>(CastlingRights::WhiteKingSide), 4,
                   piece_from(Color::White, PieceType::King),
                   "White king-side castling requires the white king on e1.");
    requires_piece(static_cast<uint8_t>(CastlingRights::WhiteKingSide), 7,
                   piece_from(Color::White, PieceType::Rook),
                   "White king-side castling requires a white rook on h1.");
    requires_piece(static_cast<uint8_t>(CastlingRights::WhiteQueenSide), 4,
                   piece_from(Color::White, PieceType::King),
                   "White queen-side castling requires the white king on e1.");
    requires_piece(static_cast<uint8_t>(CastlingRights::WhiteQueenSide), 0,
                   piece_from(Color::White, PieceType::Rook),
                   "White queen-side castling requires a white rook on a1.");
    requires_piece(static_cast<uint8_t>(CastlingRights::BlackKingSide), 60,
                   piece_from(Color::Black, PieceType::King),
                   "Black king-side castling requires the black king on e8.");
    requires_piece(static_cast<uint8_t>(CastlingRights::BlackKingSide), 63,
                   piece_from(Color::Black, PieceType::Rook),
                   "Black king-side castling requires a black rook on h8.");
    requires_piece(static_cast<uint8_t>(CastlingRights::BlackQueenSide), 60,
                   piece_from(Color::Black, PieceType::King),
                   "Black queen-side castling requires the black king on e8.");
    requires_piece(static_cast<uint8_t>(CastlingRights::BlackQueenSide), 56,
                   piece_from(Color::Black, PieceType::Rook),
                   "Black queen-side castling requires a black rook on a8.");

    result.valid = result.errors.empty();
    return result;
}

}  // namespace chesslab
