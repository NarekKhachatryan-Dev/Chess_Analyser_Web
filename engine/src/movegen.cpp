#include "chess_engine.h"

#include <algorithm>
#include <array>

namespace chesslab {
namespace {

bool in_bounds(int square) {
    return square >= 0 && square < 64;
}

void add_move(std::vector<Move>& moves, int from, int to, bool capture, bool en_passant = false,
              PieceType promotion = PieceType::None) {
    Move move;
    move.from = from;
    move.to = to;
    move.is_capture = capture;
    move.is_en_passant = en_passant;
    move.promotion = promotion;
    moves.push_back(move);
}

}  // namespace

std::vector<Move> Position::generate_legal_moves() const {
    std::vector<Move> pseudo_moves;

    for (int square = 0; square < 64; ++square) {
        const Piece piece = board[square];
        if (piece == Piece::Empty || color_of(piece) != side_to_move) {
            continue;
        }

        const PieceType type = piece_type(piece);
        const int file = file_of(square);
        const int rank = rank_of(square);
        const int direction = side_to_move == Color::White ? 1 : -1;

        switch (type) {
            case PieceType::Pawn: {
                const int one_step = square + direction * 8;
                if (in_bounds(one_step) && board[one_step] == Piece::Empty) {
                    const bool promotion = (side_to_move == Color::White && rank_of(one_step) == 7) ||
                                            (side_to_move == Color::Black && rank_of(one_step) == 0);
                    if (promotion) {
                        for (const PieceType promo : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                            add_move(pseudo_moves, square, one_step, false, false, promo);
                        }
                    } else {
                        add_move(pseudo_moves, square, one_step, false);
                    }

                    const int start_rank = side_to_move == Color::White ? 1 : 6;
                    const int two_step = square + direction * 16;
                    if (rank == start_rank && board[one_step] == Piece::Empty && board[two_step] == Piece::Empty) {
                        Move move;
                        move.from = square;
                        move.to = two_step;
                        move.is_double_pawn_push = true;
                        pseudo_moves.push_back(move);
                    }
                }

                for (int offset : {-1, 1}) {
                    const int target_file = file + offset;
                    if (target_file < 0 || target_file > 7) {
                        continue;
                    }
                    const int target = square + direction * 8 + offset;
                    if (!in_bounds(target)) {
                        continue;
                    }

                    if (target == en_passant_square) {
                        const int captured_square = target - direction * 8;
                        if (board[captured_square] != Piece::Empty && color_of(board[captured_square]) != side_to_move) {
                            add_move(pseudo_moves, square, target, true, true);
                        }
                        continue;
                    }

                    const Piece target_piece = board[target];
                    if (target_piece != Piece::Empty && color_of(target_piece) != side_to_move) {
                        const bool promotion = (side_to_move == Color::White && rank_of(target) == 7) ||
                                                (side_to_move == Color::Black && rank_of(target) == 0);
                        if (promotion) {
                            for (const PieceType promo : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight}) {
                                add_move(pseudo_moves, square, target, true, false, promo);
                            }
                        } else {
                            add_move(pseudo_moves, square, target, true);
                        }
                    }
                }
                break;
            }
            case PieceType::Knight: {
                static constexpr int offsets[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
                for (const auto& [df, dr] : offsets) {
                    const int target_file = file + df;
                    const int target_rank = rank + dr;
                    if (target_file < 0 || target_file > 7 || target_rank < 0 || target_rank > 7) {
                        continue;
                    }
                    const int target = to_index(target_file, target_rank);
                    const Piece target_piece = board[target];
                    if (target_piece == Piece::Empty || color_of(target_piece) != side_to_move) {
                        add_move(pseudo_moves, square, target, target_piece != Piece::Empty);
                    }
                }
                break;
            }
            case PieceType::Bishop:
            case PieceType::Rook:
            case PieceType::Queen: {
                static constexpr int directions[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
                for (const auto& [df, dr] : directions) {
                    bool allowed = false;
                    switch (type) {
                        case PieceType::Bishop:
                            allowed = (df != 0 && dr != 0);
                            break;
                        case PieceType::Rook:
                            allowed = (df == 0 || dr == 0);
                            break;
                        case PieceType::Queen:
                            allowed = true;
                            break;
                        default:
                            break;
                    }
                    if (!allowed) {
                        continue;
                    }

                    int x = file + df;
                    int y = rank + dr;
                    while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                        const int target = to_index(x, y);
                        const Piece target_piece = board[target];
                        if (target_piece == Piece::Empty) {
                            add_move(pseudo_moves, square, target, false);
                        } else {
                            if (color_of(target_piece) != side_to_move) {
                                add_move(pseudo_moves, square, target, true);
                            }
                            break;
                        }
                        x += df;
                        y += dr;
                    }
                }
                break;
            }
            case PieceType::King: {
                static constexpr int directions[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
                for (const auto& [df, dr] : directions) {
                    const int target_file = file + df;
                    const int target_rank = rank + dr;
                    if (target_file < 0 || target_file > 7 || target_rank < 0 || target_rank > 7) {
                        continue;
                    }
                    const int target = to_index(target_file, target_rank);
                    const Piece target_piece = board[target];
                    if (target_piece == Piece::Empty || color_of(target_piece) != side_to_move) {
                        add_move(pseudo_moves, square, target, target_piece != Piece::Empty);
                    }
                }

                const bool white_king_side = side_to_move == Color::White && (castling_rights & static_cast<uint8_t>(CastlingRights::WhiteKingSide)) != 0;
                const bool white_queen_side = side_to_move == Color::White && (castling_rights & static_cast<uint8_t>(CastlingRights::WhiteQueenSide)) != 0;
                const bool black_king_side = side_to_move == Color::Black && (castling_rights & static_cast<uint8_t>(CastlingRights::BlackKingSide)) != 0;
                const bool black_queen_side = side_to_move == Color::Black && (castling_rights & static_cast<uint8_t>(CastlingRights::BlackQueenSide)) != 0;

                if (white_king_side && square == 4 && board[5] == Piece::Empty && board[6] == Piece::Empty && board[7] == piece_from(Color::White, PieceType::Rook) &&
                    !is_square_attacked(*this, 4, Color::Black) && !is_square_attacked(*this, 5, Color::Black) && !is_square_attacked(*this, 6, Color::Black)) {
                    Move castle_move;
                    castle_move.from = 4;
                    castle_move.to = 6;
                    castle_move.is_castle = true;
                    castle_move.is_king_side_castle = true;
                    pseudo_moves.push_back(castle_move);
                }
                if (white_queen_side && square == 4 && board[1] == Piece::Empty && board[2] == Piece::Empty && board[3] == Piece::Empty && board[0] == piece_from(Color::White, PieceType::Rook) &&
                    !is_square_attacked(*this, 4, Color::Black) && !is_square_attacked(*this, 3, Color::Black) && !is_square_attacked(*this, 2, Color::Black)) {
                    Move castle_move;
                    castle_move.from = 4;
                    castle_move.to = 2;
                    castle_move.is_castle = true;
                    castle_move.is_queen_side_castle = true;
                    pseudo_moves.push_back(castle_move);
                }
                if (black_king_side && square == 60 && board[61] == Piece::Empty && board[62] == Piece::Empty && board[63] == piece_from(Color::Black, PieceType::Rook) &&
                    !is_square_attacked(*this, 60, Color::White) && !is_square_attacked(*this, 61, Color::White) && !is_square_attacked(*this, 62, Color::White)) {
                    Move castle_move;
                    castle_move.from = 60;
                    castle_move.to = 62;
                    castle_move.is_castle = true;
                    castle_move.is_king_side_castle = true;
                    pseudo_moves.push_back(castle_move);
                }
                if (black_queen_side && square == 60 && board[57] == Piece::Empty && board[58] == Piece::Empty && board[59] == Piece::Empty && board[56] == piece_from(Color::Black, PieceType::Rook) &&
                    !is_square_attacked(*this, 60, Color::White) && !is_square_attacked(*this, 59, Color::White) && !is_square_attacked(*this, 58, Color::White)) {
                    Move castle_move;
                    castle_move.from = 60;
                    castle_move.to = 58;
                    castle_move.is_castle = true;
                    castle_move.is_queen_side_castle = true;
                    pseudo_moves.push_back(castle_move);
                }
                break;
            }
            default:
                break;
        }
    }

    std::vector<Move> legal_moves;
    for (const Move& move : pseudo_moves) {
        Position candidate = *this;
        candidate.make_move(move);
        if (!candidate.is_in_check(side_to_move)) {
            legal_moves.push_back(move);
        }
    }

    return legal_moves;
}

}  // namespace chesslab
