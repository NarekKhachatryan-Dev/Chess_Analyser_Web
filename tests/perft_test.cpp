#include "chess_engine.h"

#include <doctest/doctest.h>

#include <cstdint>
#include <string>
#include <vector>

namespace {

struct PerftCase {
    const char* name;
    const char* fen;
    std::vector<uint64_t> expected;
};

uint64_t run_perft(chesslab::Position& position, int depth) {
    if (depth == 0) {
        return 1;
    }

    const auto moves = position.generate_legal_moves();
    if (depth == 1) {
        return static_cast<uint64_t>(moves.size());
    }

    uint64_t nodes = 0;
    for (const auto& move : moves) {
        chesslab::Position child = position;
        child.make_move(move);
        nodes += run_perft(child, depth - 1);
    }
    return nodes;
}

void check_perft_case(const PerftCase& test_case) {
    CAPTURE(test_case.name);
    auto position = chesslab::Position::from_fen(test_case.fen);
    for (size_t index = 0; index < test_case.expected.size(); ++index) {
        CAPTURE(index + 1);
        CHECK(run_perft(position, static_cast<int>(index + 1)) == test_case.expected[index]);
    }
}

}  // namespace

TEST_CASE("perft start position") {
    check_perft_case({
        "start position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {20, 400, 8902, 197281, 4865609},
    });
}

TEST_CASE("perft Kiwipete") {
    check_perft_case({
        "Kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        {48, 2039, 97862, 4085603},
    });
}

TEST_CASE("perft position 3") {
    check_perft_case({
        "position 3",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        {14, 191, 2812, 43238, 674624},
    });
}

TEST_CASE("perft position 4") {
    check_perft_case({
        "position 4",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        {6, 264, 9467, 422333},
    });
}

TEST_CASE("perft color-mirrored position 4") {
    check_perft_case({
        "color-mirrored position 4",
        "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
        {6, 264, 9467, 422333},
    });
}

TEST_CASE("perft position 5") {
    check_perft_case({
        "position 5",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        {44, 1486, 62379, 2103487},
    });
}
