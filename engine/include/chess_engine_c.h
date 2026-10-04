#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const char* solve_depth(const char* fen, int mate_in);
const char* validate(const char* fen);
const char* perft(const char* fen, int depth);
void free_json(const char* value);

#ifdef __cplusplus
}
#endif
