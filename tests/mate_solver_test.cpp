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

TEST_CASE("stalemate avoidance") {
    const auto result = solve_fen("8/8/2Q5/3B4/1K6/2P5/Nk6/2R5 w - - 0 1", 2);
    CHECK(result.status == chesslab::SolveStatus::MateFound);
    CHECK(result.length == 2);
    REQUIRE(result.line.size() == 3);
    CHECK(chesslab::move_to_uci(result.line[0]) == "d5h1");
    CHECK(chesslab::move_to_uci(result.line[1]) == "b2a2");
    CHECK(chesslab::move_to_uci(result.line[2]) == "c6g2");
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
    const auto result = solve_fen("7k/8/5KQ1/8/8/8/8/8 w - - 0 1", 1);
    CHECK(result.status == chesslab::SolveStatus::MateFound);
    CHECK(result.length == 1);
    REQUIRE_FALSE(result.line.empty());
    CHECK(chesslab::move_to_uci(result.line.front()) == "g6g7");

    const auto no_mate = solve_fen("8/8/8/8/8/8/7K/k7 w - - 0 1", 1);
    CHECK(no_mate.status == chesslab::SolveStatus::NoMateWithinN);
}

TEST_CASE("side to move with no legal moves is reported explicitly") {
    const auto no_moves = solve_fen("7k/5Q2/7K/8/8/8/8/8 b - - 0 1", 1);
    CHECK(no_moves.status == chesslab::SolveStatus::NoLegalMoves);
}

TEST_CASE("cancelled and timed out searches are not reported as no mate") {
    auto position = chesslab::Position::from_fen(
        "r1b2kr1/ppppn1p1/5P1p/3P3P/4P1q1/2Q5/PB4P1/nN3RK1 w - - 1 21");
    std::atomic_bool cancelled = true;
    chesslab::SolveOptions cancel_options;
    cancel_options.cancel = &cancelled;
    const auto cancelled_result = chesslab::solve(position, 4, cancel_options);
    CHECK(cancelled_result.status == chesslab::SolveStatus::Cancelled);
    CHECK(cancelled_result.status != chesslab::SolveStatus::NoMateWithinN);

    auto timed_position = chesslab::Position::from_fen(
        "r1b2kr1/ppppn1p1/5P1p/3P3P/4P1q1/2Q5/PB4P1/nN3RK1 w - - 1 21");
    chesslab::SolveOptions timeout_options;
    timeout_options.time_limit_ms = 1;
    const auto timeout_result = chesslab::solve(timed_position, 4, timeout_options);
    CHECK(timeout_result.status != chesslab::SolveStatus::NoMateWithinN);
}

TEST_CASE("no mate within requested depth is proven") {
    const auto no_mate = solve_fen("8/8/8/8/8/8/7K/k7 w - - 0 1", 3);
    CHECK(no_mate.status == chesslab::SolveStatus::NoMateWithinN);
}
