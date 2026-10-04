# ChessSolver: mate-in-N solver (C++ engine + web). University diploma project.
Title: "Շախմատային խնդիրների լուծման համակարգի ստեղծում և իրականացում"
Correctness, clean architecture, tests and code I can explain at my defense matter most.

## Product
Input: a position (FEN) and N. Question: can the side to move force checkmate within N of its own moves against any defence? If yes, show the shortest mating line.
Result is exactly one of: MateFound(length, line) | NoMateWithinN (proven by exhaustive search) | Cancelled/TimedOut (never reported as "no mate"). If the side to move has no legal moves, say so.
Web app only (no desktop GUI, no play mode, no evaluation): FEN input + simple board editor (piece palette, side to move, castling rights, en passant square), position validation with clear errors, Validation rules: exactly one king per side, no pawns on rank 1/8, the side NOT to move must not be in check, kings not adjacent, en passant square consistent (empty, correct rank, a pawn of the right color behind it). N input, Solve and Cancel buttons, progress ("searching N=3"), solution shown as text and stepped on the board (prev/next).

## Platform
Development machine is Windows. Everything must build and run on Windows and Linux. Give commands in PowerShell syntax.

## Layout
- engine/ : C++17 static library `chess_engine`, no GUI dependencies. Files: types, position (array-based, Piece enum), move, attacks, movegen, make/unmake, zobrist, fen, validation, mate_solver, perft. Only checkmate/stalemate rules; draw rules are not needed (FEN clocks parsed and ignored). C wrapper (extern "C") returning JSON: solve_depth(fen, n), validate(fen).
- cli/ : `chess_cli`: perft, divide, solve <fen> <n>, bench.
- web/ : plain JavaScript (ES modules), HTML, CSS; no framework, no bundler. Engine compiled with Emscripten to WASM and run in a Web Worker. Must work on any static server.
- tests/ : doctest via CTest; a Node.js test (node:test) for the WASM build.
- assets/ : piece images P.png p.png N.png n.png B.png b.png R.png r.png Q.png q.png K.png k.png (I provide them).
- README.md (build/run commands), docs/testing.md, docs/benchmarks.md (short).

## Solver
- Mate in N = side to move forces mate within N of its own moves; search depth 2N-1 plies. AND/OR search: attacker needs ONE move that leads to mate; defender: ALL replies must lead to mate. Defender with no legal moves: checkmate = success, stalemate = failure.
- Attacker considers ALL legal moves (not only checks). "Checks first" is move ordering only, never pruning.
- Iterative deepening N = 1..maxN so the shortest mate is found first.
- Transposition table (Zobrist hash includes side to move, castling rights, en passant square) storing "mate within d" / "no mate within d"; fixed size cap (default 64 MB). Move ordering: checks, captures, killer moves. Cancel flag and optional time limit.
- Displayed line: attacker plays a mating move; the defender plays the reply whose shortest mate is the LONGEST.
- Web: the Worker loops n = 1..maxN calling solve_depth, posts progress after each n; Cancel = worker.terminate() + respawn.

## Correctness gate
Perft must pass exactly (already passing, keep as CTest tests, never change the numbers):
- Start: 20, 400, 8902, 197281, 4865609
- `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`: 48, 2039, 97862, 4085603
- `8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1`: 14, 191, 2812, 43238, 674624
- `r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1`: 6, 264, 9467, 422333 (and its color-mirrored version)
- `rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8`: 44, 1486, 62379, 2103487
Solver tests: do NOT invent positions from memory. Take mate-in-1/2/3 problems from the Lichess open puzzle database (themes mateIn1/mateIn2/mateIn3; its FEN is the position before the opponent's first move) and cross-check our results against Stockfish (`go mate N`) with a script. Also test: "no mate in N" cases, a stalemate trap (a stalemating move must not count as mate), en passant and castling edge cases, validation. Never change an expected result to make a test pass; investigate the disagreement.

## Phases (stop after each, say how to verify, wait for "continue")
1-2. Skeleton and engine core with perft: DONE, do not redo.
3. Cleanup to match this spec (remove SFML/desktop, play/evaluation/search code), checkmate/stalemate detection, position validation, tests.
4. Mate solver + chess_cli. Add each feature (plain AND/OR, iterative deepening, ordering, transposition table) as a separate step and record nodes/time in docs/benchmarks.md.
5. WASM build + web app + Node test.
6. README, docs, final cleanup (no dead code, no unused files).

## Rules
Small steps, zero compiler warnings, no duplicated logic, no dead code, do not invent library APIs, ask me when something is ambiguous. Perft depth 5 from the start position should take only a few seconds in a Release build.