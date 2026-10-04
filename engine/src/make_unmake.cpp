#include "chess_engine.h"

namespace chesslab {

void Position::make_move(const Move& move) {
    if (move.from < 0 || move.from >= 64 || move.to < 0 || move.to >= 64) {
        return;
    }

    const Piece moving_piece = board[move.from];
    if (moving_piece == Piece::Empty) {
        return;
    }

    UndoState state;
    state.from = move.from;
    state.to = move.to;
    state.moved_piece = moving_piece;
    state.captured_square = move.to;
    state.captured_piece = Piece::Empty;
    state.en_passant_square = en_passant_square;
    state.castling_rights = castling_rights;
    state.halfmove_clock = halfmove_clock;
    state.fullmove_count = fullmove_count;
    state.was_en_passant = move.is_en_passant;

    const Color mover = color_of(moving_piece);
    const int direction = mover == Color::White ? 1 : -1;

    if (move.is_en_passant) {
        state.captured_square = move.to - direction * 8;
        state.captured_piece = board[state.captured_square];
    } else {
        state.captured_piece = board[move.to];
    }

    const auto clear_rights = [&](bool white_king_side, bool white_queen_side, bool black_king_side, bool black_queen_side) {
        if (white_king_side) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::WhiteKingSide);
        }
        if (white_queen_side) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::WhiteQueenSide);
        }
        if (black_king_side) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::BlackKingSide);
        }
        if (black_queen_side) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::BlackQueenSide);
        }
    };

    if (piece_type(moving_piece) == PieceType::King) {
        if (mover == Color::White) {
            clear_rights(true, true, false, false);
        } else {
            clear_rights(false, false, true, true);
        }
    }

    if (piece_type(moving_piece) == PieceType::Rook) {
        const int rook_from = move.from;
        if (rook_from == 0) {
            clear_rights(false, true, false, false);
        } else if (rook_from == 7) {
            clear_rights(true, false, false, false);
        } else if (rook_from == 56) {
            clear_rights(false, false, false, true);
        } else if (rook_from == 63) {
            clear_rights(false, false, true, false);
        }
    }

    if (state.captured_piece != Piece::Empty && piece_type(state.captured_piece) == PieceType::Rook) {
        if (move.to == 0) {
            clear_rights(false, true, false, false);
        } else if (move.to == 7) {
            clear_rights(true, false, false, false);
        } else if (move.to == 56) {
            clear_rights(false, false, false, true);
        } else if (move.to == 63) {
            clear_rights(false, false, true, false);
        }
    }

    if (move.is_castle) {
        const bool king_side = move.is_king_side_castle;
        if (mover == Color::White) {
            const int rook_from = king_side ? 7 : 0;
            const int rook_to = king_side ? 5 : 3;
            board[rook_from] = Piece::Empty;
            board[rook_to] = piece_from(Color::White, PieceType::Rook);
            clear_rights(true, true, false, false);
        } else {
            const int rook_from = king_side ? 63 : 56;
            const int rook_to = king_side ? 61 : 59;
            board[rook_from] = Piece::Empty;
            board[rook_to] = piece_from(Color::Black, PieceType::Rook);
            clear_rights(false, false, true, true);
        }
    }

    board[move.from] = Piece::Empty;
    if (move.is_en_passant) {
        board[state.captured_square] = Piece::Empty;
    }
    board[move.to] = moving_piece;
    if (move.promotion != PieceType::None) {
        board[move.to] = piece_from(mover, move.promotion);
    }

    if (move.is_double_pawn_push) {
        en_passant_square = (move.from + move.to) / 2;
    } else {
        en_passant_square = -1;
    }

    if (piece_type(moving_piece) == PieceType::Pawn || state.captured_piece != Piece::Empty) {
        halfmove_clock = 0;
    } else {
        ++halfmove_clock;
    }

    if (mover == Color::Black) {
        ++fullmove_count;
    }

    history.push_back(state);
    side_to_move = opposite(mover);
}

void Position::unmake_move() {
    if (history.empty()) {
        return;
    }

    const UndoState state = history.back();
    history.pop_back();

    board[state.from] = state.moved_piece;
    if (state.was_en_passant) {
        board[state.to] = Piece::Empty;
        board[state.captured_square] = state.captured_piece;
    } else {
        board[state.to] = state.captured_piece != Piece::Empty ? state.captured_piece : Piece::Empty;
    }

    if (piece_type(state.moved_piece) == PieceType::King && state.from == 4 && (state.to == 6 || state.to == 2)) {
        const bool king_side = state.to == 6;
        const int rook_from = king_side ? 5 : 3;
        const int rook_to = king_side ? 7 : 0;
        board[rook_from] = Piece::Empty;
        board[rook_to] = piece_from(Color::White, PieceType::Rook);
    } else if (piece_type(state.moved_piece) == PieceType::King && state.from == 60 && (state.to == 62 || state.to == 58)) {
        const bool king_side = state.to == 62;
        const int rook_from = king_side ? 61 : 59;
        const int rook_to = king_side ? 63 : 56;
        board[rook_from] = Piece::Empty;
        board[rook_to] = piece_from(Color::Black, PieceType::Rook);
    }

    en_passant_square = state.en_passant_square;
    castling_rights = state.castling_rights;
    halfmove_clock = state.halfmove_clock;
    fullmove_count = state.fullmove_count;
    side_to_move = color_of(state.moved_piece);
}

}  // namespace chesslab
