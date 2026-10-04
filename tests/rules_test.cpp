#include "chess_engine.h"

#include <doctest/doctest.h>

TEST_CASE("checkmate and stalemate are distinguished") {
    const auto checkmate = chesslab::Position::from_fen("7k/6Q1/5K2/8/8/8/8/8 b - - 0 1");
    CHECK(chesslab::is_checkmate(checkmate));
    CHECK_FALSE(chesslab::is_stalemate(checkmate));

    const auto stalemate = chesslab::Position::from_fen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    CHECK_FALSE(chesslab::is_checkmate(stalemate));
    CHECK(chesslab::is_stalemate(stalemate));
}

TEST_CASE("valid positions pass validation") {
    const auto result = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1"));
    CHECK(result.valid);
    CHECK(result.errors.empty());
}

TEST_CASE("validation reports required position errors") {
    const auto missing_white_king = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/8 w - - 0 1"));
    CHECK_FALSE(missing_white_king.valid);

    const auto invalid_pawn = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K2P w - - 0 1"));
    CHECK_FALSE(invalid_pawn.valid);

    const auto missing_black_king = chesslab::validate_position(
        chesslab::Position::from_fen("8/8/8/8/8/8/8/4K3 w - - 0 1"));
    CHECK_FALSE(missing_black_king.valid);

    const auto adjacent_kings = chesslab::validate_position(
        chesslab::Position::from_fen("8/8/8/8/8/8/4k3/4K3 w - - 0 1"));
    CHECK_FALSE(adjacent_kings.valid);

    const auto side_not_to_move_in_check = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/4R3/4K3 w - - 0 1"));
    CHECK_FALSE(side_not_to_move_in_check.valid);
}

TEST_CASE("validation rejects duplicate kings") {
    const auto duplicate_white_kings = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/3KK3 w - - 0 1"));
    CHECK_FALSE(duplicate_white_kings.valid);

    const auto duplicate_black_kings = chesslab::validate_position(
        chesslab::Position::from_fen("3kk3/8/8/8/8/8/8/4K3 w - - 0 1"));
    CHECK_FALSE(duplicate_black_kings.valid);
}

TEST_CASE("en passant validation rejects each inconsistent component") {
    const auto wrong_rank = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K3 w - d4 0 1"));
    CHECK_FALSE(wrong_rank.valid);

    const auto occupied_square = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/3P4/3p4/8/8/4K3 w - d5 0 1"));
    CHECK_FALSE(occupied_square.valid);

    const auto wrong_color_pawn = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/3p4/8/8/8/4K3 b - d3 0 1"));
    CHECK_FALSE(wrong_color_pawn.valid);
}

TEST_CASE("en passant validation checks square, rank, and pawn") {
    const auto valid = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1"));
    CHECK(valid.valid);

    const auto invalid = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/4P3/8/8/8/4K3 w - d6 0 1"));
    CHECK_FALSE(invalid.valid);
}

TEST_CASE("castling rights require their king and rook home squares") {
    const auto valid = chesslab::validate_position(
        chesslab::Position::from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"));
    CHECK(valid.valid);

    const auto white_king_side_missing_rook = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K3 w K - 0 1"));
    CHECK_FALSE(white_king_side_missing_rook.valid);

    const auto white_queen_side_missing_rook = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K3 w Q - 0 1"));
    CHECK_FALSE(white_queen_side_missing_rook.valid);

    const auto black_king_side_missing_rook = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K3 w k - 0 1"));
    CHECK_FALSE(black_king_side_missing_rook.valid);

    const auto black_queen_side_missing_rook = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K3 w q - 0 1"));
    CHECK_FALSE(black_queen_side_missing_rook.valid);
}
