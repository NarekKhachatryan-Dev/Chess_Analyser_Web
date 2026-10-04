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
`./build/cli/chess_cli bench`.

To cross-check all 85 real Lichess fixtures against Stockfish on Windows:

```powershell
python .\scripts\compare_solver_stockfish.py `
  -ChessCli .\build\cli\chess_cli.exe `
  -Stockfish "$env:USERPROFILE\AppData\Local\Microsoft\WinGet\Links\stockfish.exe"
```

On Linux, install Python 3 and Stockfish, then use the portable comparator:

```bash
python3 scripts/compare_solver_stockfish.py \
  --chess-cli ./build/cli/chess_cli \
  --stockfish stockfish
```

The comparator reads [lichess_mate_puzzles.tsv](<C:/Users/Narek/OneDrive/Bureaublad/Visual Studio Code/Chess_Analyser_Diplomayin/tests/data/lichess_mate_puzzles.tsv>) and reports every disagreement before returning a non-zero exit status.
