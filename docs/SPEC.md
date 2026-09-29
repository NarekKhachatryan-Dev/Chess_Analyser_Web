# Project spec: full-rules chess with analyzer, desktop + web (university diploma project)

I am a final-year IT student. This is my diploma project. Correctness, clean architecture, tests, measurable results and code I can explain at my defense matter more than cleverness. Name the project "ChessLab" (or ask me if you prefer another name).

## Product
1. Full, correct chess rules: all piece moves, castling (all conditions), en passant, promotion (Q/R/B/N), check, checkmate, stalemate, draw by insufficient material, fifty-move rule, threefold repetition.
2. Two modes, switchable by a button, on BOTH desktop and web:
   - Play mode: human vs human and human vs engine (selectable depth); click-to-move; legal-move highlights; promotion picker; last-move highlight; undo; new game; game-over message.
   - Analysis mode: position editor (place/remove pieces from a palette, side to move, castling rights), FEN import/export, position validation (exactly one king per side, no pawns on rank 1/8, side NOT to move not in check, kings not adjacent) with a clear error message, then run the engine: best move, score (centipawns or "mate in N"), principal variation, depth selector, cancel button.
3. The UI must never freeze: desktop uses a background thread with a cancel flag; web uses a Web Worker.

## Repository layout (strict, one responsibility per directory)
- engine/ : C++17 static library `chess_engine`. NO dependency on SFML, threads-of-GUI or Emscripten. Separate files per concern: types, board/position (array-based, Piece enum, NOT polymorphic piece objects), move, movegen, make/unmake, attacks, zobrist, fen, validation, rules/result detection, evaluation (material + piece-square tables), search (negamax + alpha-beta, iterative deepening, transposition table with Zobrist hashing, quiescence, MVV-LVA + killer-move ordering, mate scoring, cancel flag, optional time limit), perft.
- engine/include/ : one small public API header using plain types (FEN string in, structs out). Also a C wrapper (extern "C") that returns JSON strings, used for WASM.
- cli/ : `chess_cli` executable: `perft <fen> <depth>`, `divide`, `analyze <fen> <depth>`, `bench` (fixed set of positions, prints nodes and nodes/sec). Used for thesis benchmarks.
- desktop/ : SFML 3 GUI executable `chess_gui`, depends on chess_engine. Split into files: app, board_view, input, mode_play, mode_analysis, position_editor, analysis_worker (thread).
- web/ : static web app in plain JavaScript (ES modules), HTML, CSS. No framework and no bundler. Files: index.html, main.js, board.js (rendering + input), play_mode.js, analysis_mode.js, editor.js, engine_worker.js (Web Worker that loads the WASM module), style.css. The engine is compiled from engine/ with Emscripten into engine.wasm/engine.js via a CMake target (or a script) using the C wrapper. The whole web app must run from any static file server (`python3 -m http.server` in web/).
- tests/ : C++ tests with doctest (FetchContent) registered in CTest; plus a small Node.js test (built-in `node:test`) that loads the WASM build and checks perft and a mate-in-2.
- assets/ : piece images named P.png p.png N.png n.png B.png b.png R.png r.png Q.png q.png K.png k.png, and a font. I will add the files. CMake copies assets next to the desktop executable so it runs from any working directory. The web app uses the same images (copied or referenced by the web build).
- docs/ : SPEC.md (this file), architecture.md (modules, data structures, why), testing.md (perft results, test list), benchmarks.md (nodes/sec, search improvements measured one by one), and a short user guide.
- .github/workflows/ci.yml : build + ctest on Ubuntu; a separate job that builds the WASM target with Emscripten and runs the Node test.
- Root: CMakeLists.txt, README.md, .gitignore, .clang-format, LICENSE placeholder.

## Build requirements
- CMake >= 3.16, C++17, -Wall -Wextra -Wpedantic, zero warnings.
- No system SFML: fetch SFML 3.0.x with FetchContent pinned to an exact release tag; use the SFML 3 API (check the official docs; never guess function names). List the Ubuntu system packages SFML needs in README.
- This must work from a clean clone: `cmake -S . -B build && cmake --build build -j && ctest --test-dir build --output-on-failure`.
- Web build: documented commands using emcmake/emmake (Emscripten).

## Correctness gate (most important)
Implement perft and do NOT continue to the next phase until these pass exactly:
- Start position: d1=20, d2=400, d3=8902, d4=197281, d5=4865609
- Kiwipete `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`: 48, 2039, 97862, 4085603
- `8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1`: 14, 191, 2812, 43238, 674624
- `r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1`: 6, 264, 9467, 422333
- `rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8`: 44, 1486, 62379, 2103487
Also test: FEN round trip, checkmate, stalemate, insufficient material, fifty-move, threefold repetition, en passant edge cases (including the horizontally pinned pawn), castling through/out of/into check, promotion with capture, position validation, mate-in-1 and mate-in-2 puzzles that the search must solve. If perft mismatches, use perft divide to locate the faulty move.

## Phases (stop after each, summarize what you did and the design choices, wait for me to say "continue")
1. Skeleton: repo layout, root CMake with all targets configured, FetchContent for SFML and doctest, an empty-board desktop window, a dummy test, .gitignore, README stub. Must build.
2. Engine core: position, FEN, attacks, movegen, make/unmake, perft. Pass ALL perft numbers.
3. Rules: result detection, draws, position validation, tests.
4. Search and evaluation, `chess_cli` (perft, divide, analyze, bench). Add each search feature (alpha-beta, iterative deepening, quiescence, ordering, transposition table) as a separate commit-sized step and record nodes/time in docs/benchmarks.md after each one.
5. Desktop GUI: play mode, analysis mode with position editor, mode-switch button, analysis thread.
6. WASM build + web app (play mode, analysis mode, editor, Web Worker), Node test for WASM.
7. CI, docs (architecture, testing, benchmarks, user guide), README, cleanup (no dead code, no unused files, no silent TODOs).

## Rules for working
- Small, reviewable steps. Short comments only for non-obvious logic.
- No duplicated logic. Do not invent library APIs; check docs or ask me.
- If two requirements conflict or something is ambiguous, ask me instead of choosing silently.
- Correctness first, then speed: perft depth 5 from the start position should take only a few seconds in a Release build.
- After each phase, tell me exactly which commands to run to verify it.