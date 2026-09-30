#include "chess_engine.h"

#include <iostream>
#include <string>

namespace {

uint64_t count_nodes(chesslab::Position& position, int depth) {
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
        nodes += count_nodes(child, depth - 1);
    }
    return nodes;
}

char promotion_char(chesslab::PieceType promotion) {
    switch (promotion) {
        case chesslab::PieceType::Queen:
            return 'q';
        case chesslab::PieceType::Rook:
            return 'r';
        case chesslab::PieceType::Bishop:
            return 'b';
        case chesslab::PieceType::Knight:
            return 'n';
        default:
            return '\0';
    }
}

std::string move_to_coordinate(const chesslab::Move& move) {
    std::string result;
    result += static_cast<char>('a' + chesslab::file_of(move.from));
    result += static_cast<char>('1' + chesslab::rank_of(move.from));
    result += static_cast<char>('a' + chesslab::file_of(move.to));
    result += static_cast<char>('1' + chesslab::rank_of(move.to));
    if (const char promotion = promotion_char(move.promotion); promotion != '\0') {
        result += promotion;
    }
    return result;
}

int run_divide(const std::string& fen, int depth) {
    if (depth < 1) {
        std::cerr << "divide depth must be at least 1\n";
        return 1;
    }

    chesslab::Position position;
    try {
        position = chesslab::Position::from_fen(fen);
    } catch (const std::exception& error) {
        std::cerr << "invalid FEN: " << error.what() << '\n';
        return 1;
    }

    for (const auto& move : position.generate_legal_moves()) {
        chesslab::Position child = position;
        child.make_move(move);
        std::cout << move_to_coordinate(move) << ": " << count_nodes(child, depth - 1) << '\n';
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc >= 2 && std::string(argv[1]) == "divide") {
        if (argc != 4) {
            std::cerr << "usage: chess_cli divide \"<fen>\" <depth>\n";
            return 1;
        }

        try {
            const int depth = std::stoi(argv[3]);
            return run_divide(argv[2], depth);
        } catch (const std::exception& error) {
            std::cerr << "invalid depth: " << error.what() << '\n';
            return 1;
        }
    }

    std::cout << chesslab::kProjectName << " CLI\n";
    return argc == 1 ? 0 : 1;
}
