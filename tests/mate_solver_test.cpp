#include "chess_engine.h"

#include <doctest/doctest.h>

namespace {

chesslab::SolveResult solve_fen(const char* fen, int max_mate_in) {
    auto position = chesslab::Position::from_fen(fen);
    return chesslab::solve(position, max_mate_in);
}

}  // namespace

TEST_CASE("real Lichess mate-in-1 puzzle yxtqU") {
    const auto result = solve_fen("5k2/r3qp2/6p1/1P6/8/1BQ1P3/2P2P2/2K5 w - - 4 41", 1);
    CHECK(result.status == chesslab::SolveStatus::MateFound);
    CHECK(result.length == 1);
    REQUIRE_FALSE(result.line.empty());
    CHECK(chesslab::move_to_uci(result.line.front()) == "c3h8");
}

TEST_CASE("real Lichess mate-in-2 puzzle oiG01") {
    const auto result = solve_fen("2kr4/2p3p1/2P1b1Np/1rn5/8/5P2/1B4PP/R5K1 w - - 0 28", 2);
    CHECK(result.status == chesslab::SolveStatus::MateFound);
    CHECK(result.length == 2);
    REQUIRE_FALSE(result.line.empty());
    CHECK(chesslab::move_to_uci(result.line.front()) == "a1a8");
}

TEST_CASE("real Lichess mate-in-3 puzzle UjZa9") {
    const auto result = solve_fen("6rk/p6p/1p2Qpr1/8/3PRP2/q5P1/7P/4R1K1 b - - 0 27", 3);
    CHECK(result.status == chesslab::SolveStatus::MateFound);
    CHECK(result.length == 3);
    REQUIRE_FALSE(result.line.empty());
    CHECK(chesslab::move_to_uci(result.line.front()) == "g6g3");
}

TEST_CASE("optimized solver preserves the real puzzle result") {
    auto position = chesslab::Position::from_fen(
        "6rk/p6p/1p2Qpr1/8/3PRP2/q5P1/7P/4R1K1 b - - 0 27");
    chesslab::SolveOptions options;
    options.use_move_ordering = true;
    options.use_transposition_table = true;
    const auto result = chesslab::solve(position, 3, options);
    CHECK(result.status == chesslab::SolveStatus::MateFound);
    CHECK(result.length == 3);
    REQUIRE_FALSE(result.line.empty());
    CHECK(chesslab::move_to_uci(result.line.front()) == "g6g3");
}

TEST_CASE("solver distinguishes stalemate from checkmate") {
    const auto result = solve_fen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1", 1);
    CHECK(result.status == chesslab::SolveStatus::NoLegalMoves);

    const auto no_mate = solve_fen("7k/5Q2/7K/8/8/8/8/8 b - - 0 1", 1);
    CHECK(no_mate.status == chesslab::SolveStatus::NoLegalMoves);
}
