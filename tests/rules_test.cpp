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
    const auto result = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1"));
    CHECK(result.valid);
    CHECK(result.errors.empty());

    const auto invalid_pawn = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/8/8/8/8/4K2P w - - 0 1"));
    CHECK_FALSE(invalid_pawn.valid);

    const auto adjacent_kings = chesslab::validate_position(
        chesslab::Position::from_fen("8/8/8/8/8/8/4k3/4K3 w - - 0 1"));
    CHECK_FALSE(adjacent_kings.valid);
}

TEST_CASE("en passant validation checks square, rank, and pawn") {
    const auto valid = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1"));
    CHECK(valid.valid);

    const auto invalid = chesslab::validate_position(
        chesslab::Position::from_fen("4k3/8/8/4P3/8/8/8/4K3 w - d6 0 1"));
    CHECK_FALSE(invalid.valid);
}
