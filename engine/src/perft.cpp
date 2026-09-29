#include "chess_engine.h"

namespace chesslab {

uint64_t perft(Position& position, int depth) {
    if (depth == 0) {
        return 1ULL;
    }

    const std::vector<Move> moves = position.generate_legal_moves();
    if (depth == 1) {
        return static_cast<uint64_t>(moves.size());
    }

    uint64_t total = 0;
    for (const Move& move : moves) {
        Position next = position;
        next.make_move(move);
        total += perft(next, depth - 1);
    }

    return total;
}

}  // namespace chesslab
