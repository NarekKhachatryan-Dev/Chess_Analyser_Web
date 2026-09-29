#include "chess_engine.h"

#include <algorithm>

namespace chesslab {
namespace {

bool is_on_board(int square) {
    return square >= 0 && square < 64;
}

bool is_empty_or_enemy(const Position& position, int square, Color by_color) {
    if (!is_on_board(square)) {
        return false;
    }
    const Piece piece = position.board[square];
    if (piece == Piece::Empty) {
        return true;
    }
    return color_of(piece) != by_color;
}

void add_move(std::vector<Move>& moves, int from, int to, Color side, bool capture, bool ep, bool castle, bool double_push, PieceType promotion = PieceType::None) {
    Move move;
    move.from = from;
    move.to = to;
    move.is_capture = capture;
    move.is_en_passant = ep;
    move.is_castle = castle;
    move.is_double_pawn_push = double_push;
    move.promotion = promotion;
    moves.push_back(move);
    (void)side;
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
                const int step = square + (direction * 8);
                if (is_on_board(step) && board[step] == Piece::Empty) {
                    const int target_rank = rank_of(step);
                    if (target_rank == 0 || target_rank == 7) {
                        add_move(pseudo_moves, square, step, side_to_move, false, false, false, false, PieceType::Queen);
                        add_move(pseudo_moves, square, step, side_to_move, false, false, false, false, PieceType::Rook);
                        add_move(pseudo_moves, square, step, side_to_move, false, false, false, false, PieceType::Bishop);
                        add_move(pseudo_moves, square, step, side_to_move, false, false, false, false, PieceType::Knight);
                    } else {
                        add_move(pseudo_moves, square, step, side_to_move, false, false, false, false);
                    }

                    const int start_rank = side_to_move == Color::White ? 1 : 6;
                    if (rank == start_rank) {
                        const int double_step = square + (direction * 16);
                        if (is_on_board(double_step) && board[double_step] == Piece::Empty) {
                            add_move(pseudo_moves, square, double_step, side_to_move, false, false, false, true);
                        }
                    }
                }

                for (int offset : {-1, 1}) {
                    const int target = square + (direction * 8) + offset;
                    if (!is_on_board(target)) {
                        continue;
                    }
                    if (file + offset < 0 || file + offset > 7) {
                        continue;
                    }

                    if (en_passant_square == target) {
                        const int captured_square = target - (direction * 8);
                        if (board[captured_square] != Piece::Empty && color_of(board[captured_square]) != side_to_move) {
                            add_move(pseudo_moves, square, target, side_to_move, true, true, false, false);
                        }
                    }

                    if (board[target] != Piece::Empty && color_of(board[target]) != side_to_move) {
                        const int target_rank = rank_of(target);
                        if (target_rank == 0 || target_rank == 7) {
                            add_move(pseudo_moves, square, target, side_to_move, true, false, false, false, PieceType::Queen);
                            add_move(pseudo_moves, square, target, side_to_move, true, false, false, false, PieceType::Rook);
                            add_move(pseudo_moves, square, target, side_to_move, true, false, false, false, PieceType::Bishop);
                            add_move(pseudo_moves, square, target, side_to_move, true, false, false, false, PieceType::Knight);
                        } else {
                            add_move(pseudo_moves, square, target, side_to_move, true, false, false, false);
                        }
                    }
                }
                break;
            }
            case PieceType::Knight: {
                static const int offsets[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
                for (const auto& [df, dr] : offsets) {
                    const int to = square + df + (dr * 8);
                    if (!is_on_board(to)) {
                        continue;
                    }
                    const int to_file = file_of(to);
                    const int to_rank = rank_of(to);
                    if (std::abs(to_file - file) > 2 || std::abs(to_rank - rank) > 2) {
                        continue;
                    }
                    if (board[to] == Piece::Empty || color_of(board[to]) != side_to_move) {
                        add_move(pseudo_moves, square, to, side_to_move, board[to] != Piece::Empty, false, false, false);
                    }
                }
                break;
            }
            case PieceType::Bishop:
            case PieceType::Rook:
            case PieceType::Queen: {
                const int directions[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
                for (const auto& [df, dr] : directions) {
                    if ((type == PieceType::Bishop && (df == 0 || dr == 0)) || (type == PieceType::Rook && (std::abs(df) == 1 && std::abs(dr) == 1))) {
                        continue;
                    }
                    int next = square + df + dr * 8;
                    while (is_on_board(next)) {
                        const int next_file = file_of(next);
                        const int next_rank = rank_of(next);
                        if (std::abs(next_file - file) > 7 || std::abs(next_rank - rank) > 7) {
                            break;
                        }
                        if (board[next] == Piece::Empty) {
                            add_move(pseudo_moves, square, next, side_to_move, false, false, false, false);
                        } else {
                            if (color_of(board[next]) != side_to_move) {
                                add_move(pseudo_moves, square, next, side_to_move, true, false, false, false);
                            }
                            break;
                        }
                        next += df + dr * 8;
                    }
                }
                break;
            }
            case PieceType::King: {
                static const int eight[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
                for (const auto& [df, dr] : eight) {
                    const int to = square + df + dr * 8;
                    if (!is_on_board(to)) {
                        continue;
                    }
                    if (board[to] == Piece::Empty || color_of(board[to]) != side_to_move) {
                        add_move(pseudo_moves, square, to, side_to_move, board[to] != Piece::Empty, false, false, false);
                    }
                }

                const bool kingside = side_to_move == Color::White ? (castling_rights & static_cast<uint8_t>(CastlingRights::WhiteKingSide)) != 0 : (castling_rights & static_cast<uint8_t>(CastlingRights::BlackKingSide)) != 0;
                const bool queenside = side_to_move == Color::White ? (castling_rights & static_cast<uint8_t>(CastlingRights::WhiteQueenSide)) != 0 : (castling_rights & static_cast<uint8_t>(CastlingRights::BlackQueenSide)) != 0;

                if (square == to_index(4, side_to_move == Color::White ? 0 : 7) && !is_in_check(side_to_move)) {
                    if (kingside) {
                        const int rook_from = side_to_move == Color::White ? to_index(7, 0) : to_index(7, 7);
                        const int rook_to = side_to_move == Color::White ? to_index(5, 0) : to_index(5, 7);
                        const int through = side_to_move == Color::White ? to_index(5, 0) : to_index(5, 7);
                        const int beyond = side_to_move == Color::White ? to_index(6, 0) : to_index(6, 7);
                        if (board[rook_from] == (side_to_move == Color::White ? Piece::WhiteRook : Piece::BlackRook) && board[through] == Piece::Empty && board[beyond] == Piece::Empty && !is_square_attacked(*this, through, opposite(side_to_move)) && !is_square_attacked(*this, beyond, opposite(side_to_move))) {
                            add_move(pseudo_moves, square, beyond, side_to_move, false, false, true, false);
                        }
                    }
                    if (queenside) {
                        const int rook_from = side_to_move == Color::White ? to_index(0, 0) : to_index(0, 7);
                        const int rook_to = side_to_move == Color::White ? to_index(3, 0) : to_index(3, 7);
                        const int through = side_to_move == Color::White ? to_index(3, 0) : to_index(3, 7);
                        const int beyond = side_to_move == Color::White ? to_index(2, 0) : to_index(2, 7);
                        if (board[rook_from] == (side_to_move == Color::White ? Piece::WhiteRook : Piece::BlackRook) && board[to_index(1, side_to_move == Color::White ? 0 : 7)] == Piece::Empty && board[through] == Piece::Empty && board[beyond] == Piece::Empty && !is_square_attacked(*this, through, opposite(side_to_move)) && !is_square_attacked(*this, beyond, opposite(side_to_move))) {
                            add_move(pseudo_moves, square, beyond, side_to_move, false, false, true, false);
                        }
                    }
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
