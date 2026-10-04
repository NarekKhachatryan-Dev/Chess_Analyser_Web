# ChessSolver

ChessSolver is a C++17 chess mate-in-N solver with a web interface.

## Build

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The engine provides legal move generation, perft, checkmate/stalemate
detection, position validation, and an AND/OR mate solver with iterative
deepening, move ordering, and a capped transposition table.

## CLI

```powershell
.\build\cli\chess_cli.exe solve "<fen>" 3
.\build\cli\chess_cli.exe bench
```

The same commands work from a POSIX shell as
`./build/cli/chess_cli solve "<fen>" 3` and
`./build/cli/chess_cli bench`. To cross-check the real Lichess fixtures
against Stockfish on Windows:

```powershell
.\scripts\compare_solver_stockfish.ps1 `
  -ChessCli .\build\cli\chess_cli.exe `
  -Stockfish "$env:USERPROFILE\AppData\Local\Microsoft\WinGet\Links\stockfish.exe"
```
