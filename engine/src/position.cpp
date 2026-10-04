#include "chess_engine.h"

namespace chesslab {

Position::Position() {
    board.fill(Piece::Empty);
    side_to_move = Color::White;
    en_passant_square = -1;
    castling_rights = static_cast<uint8_t>(CastlingRights::WhiteKingSide | CastlingRights::WhiteQueenSide |
                                          CastlingRights::BlackKingSide | CastlingRights::BlackQueenSide);
    halfmove_clock = 0;
    fullmove_count = 1;
    history.clear();

    board[0] = piece_from(Color::White, PieceType::Rook);
    board[1] = piece_from(Color::White, PieceType::Knight);
    board[2] = piece_from(Color::White, PieceType::Bishop);
    board[3] = piece_from(Color::White, PieceType::Queen);
    board[4] = piece_from(Color::White, PieceType::King);
    board[5] = piece_from(Color::White, PieceType::Bishop);
    board[6] = piece_from(Color::White, PieceType::Knight);
    board[7] = piece_from(Color::White, PieceType::Rook);
    for (int i = 8; i < 16; ++i) {
        board[i] = piece_from(Color::White, PieceType::Pawn);
    }

    board[56] = piece_from(Color::Black, PieceType::Rook);
    board[57] = piece_from(Color::Black, PieceType::Knight);
    board[58] = piece_from(Color::Black, PieceType::Bishop);
    board[59] = piece_from(Color::Black, PieceType::Queen);
    board[60] = piece_from(Color::Black, PieceType::King);
    board[61] = piece_from(Color::Black, PieceType::Bishop);
    board[62] = piece_from(Color::Black, PieceType::Knight);
    board[63] = piece_from(Color::Black, PieceType::Rook);
    for (int i = 48; i < 56; ++i) {
        board[i] = piece_from(Color::Black, PieceType::Pawn);
    }
}

}  // namespace chesslab
