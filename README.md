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

## Phase 5 WASM and web app

PowerShell commands on Windows (run the emsdk setup in the same terminal):

```powershell
& "$env:USERPROFILE\emsdk\emsdk_env.ps1"
.\scripts\build_wasm.ps1
$env:CHESS_WASM_MODULE = (Resolve-Path .\web\chess_engine.js).Path
node --test .\tests\wasm_test.mjs
python -m http.server 8000 --directory web
```

Then open `http://127.0.0.1:8000/`. The app uses the generated WASM module
from a Web Worker and requires a static server; opening `index.html` directly
does not provide the module-loading guarantees needed by browsers.

On Linux, use the equivalent Emscripten environment setup and build commands:

```bash
source "$HOME/emsdk/emsdk_env.sh"
emcmake cmake -S . -B build_wasm -DCMAKE_BUILD_TYPE=Release
cmake --build build_wasm --target chess_engine_wasm -j
cp build_wasm/engine/chess_engine.js web/chess_engine.js
cp build_wasm/engine/chess_engine.wasm web/chess_engine.wasm
CHESS_WASM_MODULE="$PWD/web/chess_engine.js" node --test tests/wasm_test.mjs
python3 -m http.server 8000 --directory web
```
