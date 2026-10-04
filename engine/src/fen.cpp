#include "chess_engine.h"

#include <cctype>
#include <sstream>
#include <stdexcept>

namespace chesslab {
namespace {

char piece_to_char(Piece piece) {
    if (piece == Piece::Empty) {
        return ' ';
    }

    char base = ' ';
    switch (piece_type(piece)) {
        case PieceType::Pawn:
            base = 'p';
            break;
        case PieceType::Knight:
            base = 'n';
            break;
        case PieceType::Bishop:
            base = 'b';
            break;
        case PieceType::Rook:
            base = 'r';
            break;
        case PieceType::Queen:
            base = 'q';
            break;
        case PieceType::King:
            base = 'k';
            break;
        default:
            break;
    }

    if (color_of(piece) == Color::White) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(base)));
    }
    return base;
}

Piece char_to_piece(char ch) {
    const bool white = std::isupper(static_cast<unsigned char>(ch)) != 0;
    switch (std::tolower(static_cast<unsigned char>(ch))) {
        case 'p':
            return piece_from(white ? Color::White : Color::Black, PieceType::Pawn);
        case 'n':
            return piece_from(white ? Color::White : Color::Black, PieceType::Knight);
        case 'b':
            return piece_from(white ? Color::White : Color::Black, PieceType::Bishop);
        case 'r':
            return piece_from(white ? Color::White : Color::Black, PieceType::Rook);
        case 'q':
            return piece_from(white ? Color::White : Color::Black, PieceType::Queen);
        case 'k':
            return piece_from(white ? Color::White : Color::Black, PieceType::King);
        default:
            return Piece::Empty;
    }
}

std::string square_to_algebraic(int square) {
    if (square < 0 || square >= 64) {
        return "-";
    }
    return std::string(1, static_cast<char>('a' + file_of(square))) + std::string(1, static_cast<char>('1' + rank_of(square)));
}

}  // namespace

Position Position::from_fen(const std::string& fen) {
    Position pos;
    pos.board.fill(Piece::Empty);
    pos.en_passant_square = -1;
    pos.castling_rights = static_cast<uint8_t>(CastlingRights::NoCastling);
    pos.halfmove_clock = 0;
    pos.fullmove_count = 1;

    std::istringstream stream(fen);
    std::string board_part;
    std::string side_part;
    std::string castling_part;
    std::string en_passant_part;
    std::string halfmove_part;
    std::string fullmove_part;

    if (!(stream >> board_part >> side_part >> castling_part >> en_passant_part >> halfmove_part >> fullmove_part)) {
        throw std::invalid_argument("Invalid FEN string");
    }

    int file = 0;
    int rank = 7;
    for (char ch : board_part) {
        if (ch == '/') {
            --rank;
            file = 0;
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            file += ch - '0';
            continue;
        }
        if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            pos.board[to_index(file, rank)] = char_to_piece(ch);
            ++file;
        }
    }

    pos.side_to_move = side_part == "w" ? Color::White : Color::Black;

    for (char ch : castling_part) {
        switch (ch) {
            case 'K':
                pos.castling_rights |= static_cast<uint8_t>(CastlingRights::WhiteKingSide);
                break;
            case 'Q':
                pos.castling_rights |= static_cast<uint8_t>(CastlingRights::WhiteQueenSide);
                break;
            case 'k':
                pos.castling_rights |= static_cast<uint8_t>(CastlingRights::BlackKingSide);
                break;
            case 'q':
                pos.castling_rights |= static_cast<uint8_t>(CastlingRights::BlackQueenSide);
                break;
            default:
                break;
        }
    }

    if (en_passant_part != "-") {
        pos.en_passant_square = to_index(en_passant_part[0] - 'a', en_passant_part[1] - '1');
    }

    pos.halfmove_clock = std::stoi(halfmove_part);
    pos.fullmove_count = std::stoi(fullmove_part);
    return pos;
}

std::string Position::to_fen() const {
    std::string board_part;
    for (int rank = 7; rank >= 0; --rank) {
        int empty_count = 0;
        for (int file = 0; file < 8; ++file) {
            const Piece piece = board[to_index(file, rank)];
            if (piece == Piece::Empty) {
                ++empty_count;
                continue;
            }
            if (empty_count > 0) {
                board_part.push_back(static_cast<char>('0' + empty_count));
                empty_count = 0;
            }
            board_part.push_back(piece_to_char(piece));
        }
        if (empty_count > 0) {
            board_part.push_back(static_cast<char>('0' + empty_count));
        }
        if (rank > 0) {
            board_part.push_back('/');
        }
    }

    std::string castling_part;
    if ((castling_rights & static_cast<uint8_t>(CastlingRights::WhiteKingSide)) != 0) {
        castling_part.push_back('K');
    }
    if ((castling_rights & static_cast<uint8_t>(CastlingRights::WhiteQueenSide)) != 0) {
        castling_part.push_back('Q');
    }
    if ((castling_rights & static_cast<uint8_t>(CastlingRights::BlackKingSide)) != 0) {
        castling_part.push_back('k');
    }
    if ((castling_rights & static_cast<uint8_t>(CastlingRights::BlackQueenSide)) != 0) {
        castling_part.push_back('q');
    }
    if (castling_part.empty()) {
        castling_part = "-";
    }

    std::string en_passant_part = "-";
    if (en_passant_square >= 0 && en_passant_square < 64) {
        en_passant_part = square_to_algebraic(en_passant_square);
    }

    return board_part + " " + (side_to_move == Color::White ? "w" : "b") + " " + castling_part + " " + en_passant_part + " " +
           std::to_string(halfmove_clock) + " " + std::to_string(fullmove_count);
}

}  // namespace chesslab
