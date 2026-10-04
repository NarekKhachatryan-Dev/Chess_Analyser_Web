# ChessSolver

ChessSolver is a C++17 chess mate-in-N solver with a web interface.

## Build

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The engine currently provides legal move generation, perft, checkmate/stalemate
detection, and position validation.
