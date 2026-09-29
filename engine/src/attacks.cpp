#include "chess_engine.h"

#include <cmath>

namespace chesslab {

bool is_square_attacked(const Position& position, int square, Color by_color) {
    for (int from = 0; from < 64; ++from) {
        const Piece piece = position.board[from];
        if (piece == Piece::Empty || color_of(piece) != by_color) {
            continue;
        }

        const int file_diff = file_of(square) - file_of(from);
        const int rank_diff = rank_of(square) - rank_of(from);
        const int abs_file = std::abs(file_diff);
        const int abs_rank = std::abs(rank_diff);
        const PieceType type = piece_type(piece);

        switch (type) {
            case PieceType::Pawn: {
                const int direction = by_color == Color::White ? 1 : -1;
                if (rank_diff == direction && abs_file == 1) {
                    return true;
                }
                break;
            }
            case PieceType::Knight:
                if ((abs_file == 1 && abs_rank == 2) || (abs_file == 2 && abs_rank == 1)) {
                    return true;
                }
                break;
            case PieceType::King:
                if (abs_file <= 1 && abs_rank <= 1 && !(abs_file == 0 && abs_rank == 0)) {
                    return true;
                }
                break;
            case PieceType::Bishop: {
                if (abs_file == abs_rank && abs_file > 0) {
                    const int step_file = file_diff > 0 ? 1 : -1;
                    const int step_rank = rank_diff > 0 ? 1 : -1;
                    int x = file_of(from) + step_file;
                    int y = rank_of(from) + step_rank;
                    bool blocked = false;
                    while (x != file_of(square) || y != rank_of(square)) {
                        const int current = y * 8 + x;
                        if (position.board[current] != Piece::Empty) {
                            blocked = true;
                            break;
                        }
                        x += step_file;
                        y += step_rank;
                    }
                    if (!blocked) {
                        return true;
                    }
                }
                break;
            }
            case PieceType::Rook: {
                if (file_diff == 0 || rank_diff == 0) {
                    int step_file = 0;
                    int step_rank = 0;
                    if (file_diff != 0) {
                        step_file = file_diff > 0 ? 1 : -1;
                    }
                    if (rank_diff != 0) {
                        step_rank = rank_diff > 0 ? 1 : -1;
                    }
                    int x = file_of(from) + step_file;
                    int y = rank_of(from) + step_rank;
                    bool blocked = false;
                    while (x != file_of(square) || y != rank_of(square)) {
                        const int current = y * 8 + x;
                        if (position.board[current] != Piece::Empty) {
                            blocked = true;
                            break;
                        }
                        x += step_file;
                        y += step_rank;
                    }
                    if (!blocked) {
                        return true;
                    }
                }
                break;
            }
            case PieceType::Queen: {
                const bool diagonal = abs_file == abs_rank && abs_file > 0;
                const bool straight = (file_diff == 0 || rank_diff == 0) && (file_diff != 0 || rank_diff != 0);
                if (diagonal || straight) {
                    const int step_file = file_diff == 0 ? 0 : (file_diff > 0 ? 1 : -1);
                    const int step_rank = rank_diff == 0 ? 0 : (rank_diff > 0 ? 1 : -1);
                    int x = file_of(from) + step_file;
                    int y = rank_of(from) + step_rank;
                    bool blocked = false;
                    while (x != file_of(square) || y != rank_of(square)) {
                        const int current = y * 8 + x;
                        if (position.board[current] != Piece::Empty) {
                            blocked = true;
                            break;
                        }
                        x += step_file;
                        y += step_rank;
                    }
                    if (!blocked) {
                        return true;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return false;
}

bool Position::is_in_check(Color side) const {
    for (int square = 0; square < 64; ++square) {
        if (board[square] == piece_from(side, PieceType::King)) {
            return is_square_attacked(*this, square, opposite(side));
        }
    }
    return false;
}

}  // namespace chesslab
