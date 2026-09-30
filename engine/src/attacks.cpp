#include "chess_engine.h"

#include <cmath>

namespace chesslab {
namespace {

bool in_bounds(int square) {
    return square >= 0 && square < 64;
}

}  // namespace

bool is_square_attacked(const Position& position, int square, Color by_color) {
    for (int from = 0; from < 64; ++from) {
        const Piece piece = position.board[from];
        if (piece == Piece::Empty || color_of(piece) != by_color) {
            continue;
        }

        const int df = file_of(square) - file_of(from);
        const int dr = rank_of(square) - rank_of(from);
        const int abs_df = std::abs(df);
        const int abs_dr = std::abs(dr);
        const PieceType type = piece_type(piece);

        switch (type) {
            case PieceType::Pawn: {
                const int direction = by_color == Color::White ? 1 : -1;
                if (dr == direction && abs_df == 1) {
                    return true;
                }
                break;
            }
            case PieceType::Knight:
                if ((abs_df == 1 && abs_dr == 2) || (abs_df == 2 && abs_dr == 1)) {
                    return true;
                }
                break;
            case PieceType::King:
                if (abs_df <= 1 && abs_dr <= 1 && !(abs_df == 0 && abs_dr == 0)) {
                    return true;
                }
                break;
            case PieceType::Bishop: {
                if (abs_df == abs_dr && abs_df > 0) {
                    const int step_file = df > 0 ? 1 : -1;
                    const int step_rank = dr > 0 ? 1 : -1;
                    int x = file_of(from) + step_file;
                    int y = rank_of(from) + step_rank;
                    bool blocked = false;
                    while (x != file_of(square) || y != rank_of(square)) {
                        const int next = to_index(x, y);
                        if (!in_bounds(next) || position.board[next] != Piece::Empty) {
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
                if (df == 0 || dr == 0) {
                    int step_file = 0;
                    int step_rank = 0;
                    if (df != 0) {
                        step_file = df > 0 ? 1 : -1;
                    }
                    if (dr != 0) {
                        step_rank = dr > 0 ? 1 : -1;
                    }
                    int x = file_of(from) + step_file;
                    int y = rank_of(from) + step_rank;
                    bool blocked = false;
                    while (x != file_of(square) || y != rank_of(square)) {
                        const int next = to_index(x, y);
                        if (!in_bounds(next) || position.board[next] != Piece::Empty) {
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
                const bool diagonal = abs_df == abs_dr && abs_df > 0;
                const bool straight = (df == 0 || dr == 0) && (df != 0 || dr != 0);
                if (diagonal || straight) {
                    const int step_file = df == 0 ? 0 : (df > 0 ? 1 : -1);
                    const int step_rank = dr == 0 ? 0 : (dr > 0 ? 1 : -1);
                    int x = file_of(from) + step_file;
                    int y = rank_of(from) + step_rank;
                    bool blocked = false;
                    while (x != file_of(square) || y != rank_of(square)) {
                        const int next = to_index(x, y);
                        if (!in_bounds(next) || position.board[next] != Piece::Empty) {
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
    for (int sq = 0; sq < 64; ++sq) {
        if (board[sq] == piece_from(side, PieceType::King)) {
            return is_square_attacked(*this, sq, opposite(side));
        }
    }
    return false;
}

}  // namespace chesslab
