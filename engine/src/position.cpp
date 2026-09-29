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

    const PieceType type = piece_type(piece);
    char base = ' ';
    switch (type) {
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
    *this = from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

Position Position::from_fen(const std::string& fen) {
    Position position;
    position.board.fill(Piece::Empty);
    position.en_passant_square = -1;
    position.castling_rights = static_cast<uint8_t>(CastlingRights::NoCastling);
    position.halfmove_clock = 0;
    position.fullmove_count = 1;

    std::istringstream stream(fen);
    std::string board_part;
    std::string side_part;
    std::string castling_part;
    std::string en_passant_part;
    std::string halfmove_part;
    std::string fullmove_part;

    if (!(stream >> board_part >> side_part >> castling_part >> en_passant_part >> halfmove_part >> fullmove_part)) {
        throw std::invalid_argument("Invalid FEN string: missing fields");
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
            position.board[to_index(file, rank)] = char_to_piece(ch);
            ++file;
        }
    }

    position.side_to_move = (side_part == "w") ? Color::White : Color::Black;

    for (char ch : castling_part) {
        switch (ch) {
            case 'K':
                position.castling_rights |= static_cast<uint8_t>(CastlingRights::WhiteKingSide);
                break;
            case 'Q':
                position.castling_rights |= static_cast<uint8_t>(CastlingRights::WhiteQueenSide);
                break;
            case 'k':
                position.castling_rights |= static_cast<uint8_t>(CastlingRights::BlackKingSide);
                break;
            case 'q':
                position.castling_rights |= static_cast<uint8_t>(CastlingRights::BlackQueenSide);
                break;
            default:
                break;
        }
    }

    if (en_passant_part != "-") {
        const int ep_file = en_passant_part[0] - 'a';
        const int ep_rank = en_passant_part[1] - '1';
        position.en_passant_square = to_index(ep_file, ep_rank);
    }

    position.halfmove_clock = std::stoi(halfmove_part);
    position.fullmove_count = std::stoi(fullmove_part);
    return position;
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
    state.was_castle = move.is_castle;
    state.was_en_passant = move.is_en_passant;
    state.rook_from = -1;
    state.rook_to = -1;

    const int direction = side_to_move == Color::White ? 1 : -1;
    if (move.is_en_passant) {
        state.captured_square = move.to - direction * 8;
        state.captured_piece = board[state.captured_square];
    } else {
        state.captured_piece = board[move.to];
    }

    // Remove moving piece and captured pawn if needed.
    board[move.from] = Piece::Empty;
    if (move.is_en_passant) {
        board[state.captured_square] = Piece::Empty;
    }

    if (move.is_castle) {
        if (move.to == to_index(6, 0)) {
            state.rook_from = to_index(7, 0);
            state.rook_to = to_index(5, 0);
            board[state.rook_from] = Piece::Empty;
            board[state.rook_to] = Piece::WhiteRook;
        } else if (move.to == to_index(2, 0)) {
            state.rook_from = to_index(0, 0);
            state.rook_to = to_index(3, 0);
            board[state.rook_from] = Piece::Empty;
            board[state.rook_to] = Piece::WhiteRook;
        } else if (move.to == to_index(6, 7)) {
            state.rook_from = to_index(7, 7);
            state.rook_to = to_index(5, 7);
            board[state.rook_from] = Piece::Empty;
            board[state.rook_to] = Piece::BlackRook;
        } else if (move.to == to_index(2, 7)) {
            state.rook_from = to_index(0, 7);
            state.rook_to = to_index(3, 7);
            board[state.rook_from] = Piece::Empty;
            board[state.rook_to] = Piece::BlackRook;
        }
    }

    // Place the moved piece at destination.
    board[move.to] = moving_piece;
    if (move.promotion != PieceType::None) {
        board[move.to] = piece_from(side_to_move, move.promotion);
    }

    if (moving_piece == Piece::WhiteKing) {
        castling_rights &= static_cast<uint8_t>(~(CastlingRights::WhiteKingSide | CastlingRights::WhiteQueenSide));
    } else if (moving_piece == Piece::BlackKing) {
        castling_rights &= static_cast<uint8_t>(~(CastlingRights::BlackKingSide | CastlingRights::BlackQueenSide));
    }

    if (moving_piece == Piece::WhiteRook && move.from == to_index(0, 0)) {
        castling_rights &= static_cast<uint8_t>(~CastlingRights::WhiteQueenSide);
    }
    if (moving_piece == Piece::WhiteRook && move.from == to_index(7, 0)) {
        castling_rights &= static_cast<uint8_t>(~CastlingRights::WhiteKingSide);
    }
    if (moving_piece == Piece::BlackRook && move.from == to_index(0, 7)) {
        castling_rights &= static_cast<uint8_t>(~CastlingRights::BlackQueenSide);
    }
    if (moving_piece == Piece::BlackRook && move.from == to_index(7, 7)) {
        castling_rights &= static_cast<uint8_t>(~CastlingRights::BlackKingSide);
    }

    if (state.captured_piece != Piece::Empty) {
        if (state.captured_piece == Piece::WhiteRook && state.captured_square == to_index(0, 0)) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::WhiteQueenSide);
        }
        if (state.captured_piece == Piece::WhiteRook && state.captured_square == to_index(7, 0)) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::WhiteKingSide);
        }
        if (state.captured_piece == Piece::BlackRook && state.captured_square == to_index(0, 7)) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::BlackQueenSide);
        }
        if (state.captured_piece == Piece::BlackRook && state.captured_square == to_index(7, 7)) {
            castling_rights &= static_cast<uint8_t>(~CastlingRights::BlackKingSide);
        }
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

    if (side_to_move == Color::Black) {
        ++fullmove_count;
    }

    history.push_back(state);
    side_to_move = opposite(side_to_move);
}

void Position::unmake_move() {
    if (history.empty()) {
        return;
    }

    const UndoState state = history.back();
    history.pop_back();

    // Restore board state.
    board[state.from] = state.moved_piece;
    if (state.was_en_passant) {
        board[state.to] = Piece::Empty;
        board[state.captured_square] = state.captured_piece;
    } else if (state.captured_piece != Piece::Empty) {
        board[state.to] = state.captured_piece;
    } else {
        board[state.to] = Piece::Empty;
    }

    if (state.was_castle && state.rook_from != -1 && state.rook_to != -1) {
        board[state.rook_from] = board[state.rook_to];
        board[state.rook_to] = Piece::Empty;
    }

    en_passant_square = state.en_passant_square;
    castling_rights = state.castling_rights;
    halfmove_clock = state.halfmove_clock;
    fullmove_count = state.fullmove_count;
    side_to_move = opposite(side_to_move);
}

}  // namespace chesslab
