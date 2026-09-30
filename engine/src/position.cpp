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

Position::Position() {
    board.fill(Piece::Empty);
    side_to_move = Color::White;
    en_passant_square = -1;
    castling_rights = static_cast<uint8_t>(CastlingRights::WhiteKingSide | CastlingRights::WhiteQueenSide |
                                          CastlingRights::BlackKingSide | CastlingRights::BlackQueenSide);
    halfmove_clock = 0;
    fullmove_count = 1;
    history.clear();

    board[0] = piece_from(Color::White, PieceType::Rook);
    board[1] = piece_from(Color::White, PieceType::Knight);
    board[2] = piece_from(Color::White, PieceType::Bishop);
    board[3] = piece_from(Color::White, PieceType::Queen);
    board[4] = piece_from(Color::White, PieceType::King);
    board[5] = piece_from(Color::White, PieceType::Bishop);
    board[6] = piece_from(Color::White, PieceType::Knight);
    board[7] = piece_from(Color::White, PieceType::Rook);
    for (int i = 8; i < 16; ++i) {
        board[i] = piece_from(Color::White, PieceType::Pawn);
    }

    board[56] = piece_from(Color::Black, PieceType::Rook);
    board[57] = piece_from(Color::Black, PieceType::Knight);
    board[58] = piece_from(Color::Black, PieceType::Bishop);
    board[59] = piece_from(Color::Black, PieceType::Queen);
    board[60] = piece_from(Color::Black, PieceType::King);
    board[61] = piece_from(Color::Black, PieceType::Bishop);
    board[62] = piece_from(Color::Black, PieceType::Knight);
    board[63] = piece_from(Color::Black, PieceType::Rook);
    for (int i = 48; i < 56; ++i) {
        board[i] = piece_from(Color::Black, PieceType::Pawn);
    }
}

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

void Position::make_move(const Move& move) {
    if (move.from < 0 || move.from >= 64 || move.to < 0 || move.to >= 64) {
        return;
    }

    const Piece moving_piece = board[move.from];
    if (moving_piece == Piece::Empty) {
        return;
    }

    UndoState state;
    state.from = move.from;
    state.to = move.to;
    state.moved_piece = moving_piece;
    state.captured_square = move.to;
    state.captured_piece = Piece::Empty;
    state.en_passant_square = en_passant_square;
    state.castling_rights = castling_rights;
    state.halfmove_clock = halfmove_clock;
    state.fullmove_count = fullmove_count;
    state.was_en_passant = move.is_en_passant;

    const Color mover = color_of(moving_piece);
    const int direction = mover == Color::White ? 1 : -1;

    if (move.is_en_passant) {
        state.captured_square = move.to - direction * 8;
        state.captured_piece = board[state.captured_square];
    } else {
        state.captured_piece = board[move.to];
    }

    const auto clear_rights = [&](bool white_king_side, bool white_queen_side, bool black_king_side, bool black_queen_side) {
        if (white_king_side) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::WhiteKingSide);
        }
        if (white_queen_side) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::WhiteQueenSide);
        }
        if (black_king_side) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::BlackKingSide);
        }
        if (black_queen_side) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::BlackQueenSide);
        }
    };

    if (piece_type(moving_piece) == PieceType::King) {
        if (mover == Color::White) {
            clear_rights(true, true, false, false);
        } else {
            clear_rights(false, false, true, true);
        }
    }

    if (piece_type(moving_piece) == PieceType::Rook) {
        const int rook_from = move.from;
        if (rook_from == 0) {
            clear_rights(false, true, false, false);
        } else if (rook_from == 7) {
            clear_rights(true, false, false, false);
        } else if (rook_from == 56) {
            clear_rights(false, false, false, true);
        } else if (rook_from == 63) {
            clear_rights(false, false, true, false);
        }
    }

    if (state.captured_piece != Piece::Empty && piece_type(state.captured_piece) == PieceType::Rook) {
        if (move.to == 0) {
            clear_rights(false, true, false, false);
        } else if (move.to == 7) {
            clear_rights(true, false, false, false);
        } else if (move.to == 56) {
            clear_rights(false, false, false, true);
        } else if (move.to == 63) {
            clear_rights(false, false, true, false);
        }
    }

    if (move.is_castle) {
        const bool king_side = move.is_king_side_castle;
        if (mover == Color::White) {
            const int rook_from = king_side ? 7 : 0;
            const int rook_to = king_side ? 5 : 3;
            board[rook_from] = Piece::Empty;
            board[rook_to] = piece_from(Color::White, PieceType::Rook);
            clear_rights(true, true, false, false);
        } else {
            const int rook_from = king_side ? 63 : 56;
            const int rook_to = king_side ? 61 : 59;
            board[rook_from] = Piece::Empty;
            board[rook_to] = piece_from(Color::Black, PieceType::Rook);
            clear_rights(false, false, true, true);
        }
    }

    board[move.from] = Piece::Empty;
    if (move.is_en_passant) {
        board[state.captured_square] = Piece::Empty;
    }
    board[move.to] = moving_piece;
    if (move.promotion != PieceType::None) {
        board[move.to] = piece_from(mover, move.promotion);
    }

    if (move.is_double_pawn_push) {
        en_passant_square = (move.from + move.to) / 2;
    } else {
        en_passant_square = -1;
    }

    if (piece_type(moving_piece) == PieceType::Pawn || state.captured_piece != Piece::Empty) {
        halfmove_clock = 0;
    } else {
        ++halfmove_clock;
    }

    if (mover == Color::Black) {
        ++fullmove_count;
    }

    history.push_back(state);
    side_to_move = opposite(mover);
}

void Position::unmake_move() {
    if (history.empty()) {
        return;
    }

    const UndoState state = history.back();
    history.pop_back();

    board[state.from] = state.moved_piece;
    if (state.was_en_passant) {
        board[state.to] = Piece::Empty;
        board[state.captured_square] = state.captured_piece;
    } else {
        board[state.to] = state.captured_piece != Piece::Empty ? state.captured_piece : Piece::Empty;
    }

    if (piece_type(state.moved_piece) == PieceType::King && state.from == 4 && (state.to == 6 || state.to == 2)) {
        const bool king_side = state.to == 6;
        const int rook_from = king_side ? 5 : 3;
        const int rook_to = king_side ? 7 : 0;
        board[rook_from] = Piece::Empty;
        board[rook_to] = piece_from(Color::White, PieceType::Rook);
    } else if (piece_type(state.moved_piece) == PieceType::King && state.from == 60 && (state.to == 62 || state.to == 58)) {
        const bool king_side = state.to == 62;
        const int rook_from = king_side ? 61 : 59;
        const int rook_to = king_side ? 63 : 56;
        board[rook_from] = Piece::Empty;
        board[rook_to] = piece_from(Color::Black, PieceType::Rook);
    }

    en_passant_square = state.en_passant_square;
    castling_rights = state.castling_rights;
    halfmove_clock = state.halfmove_clock;
    fullmove_count = state.fullmove_count;
    side_to_move = color_of(state.moved_piece);
}

}  // namespace chesslab
