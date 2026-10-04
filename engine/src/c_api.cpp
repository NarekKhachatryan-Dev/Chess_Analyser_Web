#include "chess_engine_c.h"

#include "chess_engine.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace {

constexpr int kMaxMateIn = 5;
constexpr uint64_t kDefaultTimeLimitMs = 30000;

char* copy_json(const std::string& value) {
    char* result = static_cast<char*>(std::malloc(value.size() + 1));
    if (result == nullptr) {
        return nullptr;
    }
    std::memcpy(result, value.c_str(), value.size() + 1);
    return result;
}

std::string escape_json(const std::string& value) {
    std::string result;
    for (const char character : value) {
        if (character == '\\' || character == '"') {
            result += '\\';
        }
        result += character;
    }
    return result;
}

const char* status_name(chesslab::SolveStatus status) {
    switch (status) {
        case chesslab::SolveStatus::MateFound:
            return "MateFound";
        case chesslab::SolveStatus::NoMateWithinN:
            return "NoMateWithinN";
        case chesslab::SolveStatus::NoLegalMoves:
            return "NoLegalMoves";
        case chesslab::SolveStatus::Cancelled:
            return "Cancelled";
        case chesslab::SolveStatus::TimedOut:
            return "TimedOut";
    }
    return "Unknown";
}

std::string error_json(const std::exception& error) {
    return std::string("{\"status\":\"Error\",\"error\":\"") + escape_json(error.what()) + "\"}";
}

}  // namespace

extern "C" const char* solve_depth(const char* fen, int mate_in) {
    try {
        if (fen == nullptr) {
            throw std::invalid_argument("FEN is null");
        }
        if (mate_in < 1 || mate_in > kMaxMateIn) {
            return copy_json("{\"status\":\"Error\",\"error\":\"Mate depth must be between 1 and 5\"}");
        }
        auto position = chesslab::Position::from_fen(fen);
        chesslab::SolveOptions options;
        options.use_move_ordering = true;
        options.use_transposition_table = true;
        options.transposition_table_size_mb = 16;
        options.time_limit_ms = kDefaultTimeLimitMs;
        const auto result = chesslab::solve_depth(position, mate_in, options);
        std::ostringstream json;
        json << "{\"status\":\"" << status_name(result.status) << "\",\"length\":" << result.length
             << ",\"nodes\":" << result.nodes << ",\"elapsed_ms\":" << result.elapsed_ms << ",\"line\":[";
        for (size_t index = 0; index < result.line.size(); ++index) {
            if (index != 0) {
                json << ',';
            }
            json << '"' << chesslab::move_to_uci(result.line[index]) << '"';
        }
        json << "]}";
        return copy_json(json.str());
    } catch (const std::exception& error) {
        return copy_json(error_json(error));
    }
}

extern "C" const char* validate(const char* fen) {
    try {
        if (fen == nullptr) {
            throw std::invalid_argument("FEN is null");
        }
        const auto position = chesslab::Position::from_fen(fen);
        const auto validation = chesslab::validate_position(position);
        std::ostringstream json;
        json << "{\"valid\":" << (validation.valid ? "true" : "false") << ",\"errors\":[";
        for (size_t index = 0; index < validation.errors.size(); ++index) {
            if (index != 0) {
                json << ',';
            }
            json << '"' << escape_json(validation.errors[index]) << '"';
        }
        json << "]}";
        return copy_json(json.str());
    } catch (const std::exception& error) {
        return copy_json(error_json(error));
    }
}

extern "C" const char* perft(const char* fen, int depth) {
    try {
        if (fen == nullptr) {
            throw std::invalid_argument("FEN is null");
        }
        auto position = chesslab::Position::from_fen(fen);
        std::ostringstream json;
        json << "{\"nodes\":" << chesslab::perft(position, depth) << "}";
        return copy_json(json.str());
    } catch (const std::exception& error) {
        return copy_json(error_json(error));
    }
}

extern "C" void free_json(const char* value) {
    std::free(const_cast<char*>(value));
}
