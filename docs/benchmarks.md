# ChessSolver benchmarks

Each entry is recorded after the corresponding Phase 4 feature was built and
the full CTest suite passed. Times depend on the local Windows machine.

## Plain AND/OR search

The plain solver exhaustively searches all legal moves without move ordering,
iterative deepening, or a transposition table.

| Fixture | Requested depth | Nodes | Time |
|---|---:|---:|---:|
| Lichess mate-in-1 `yxtqU` | 1 | 24 | 1 ms |
| Lichess mate-in-2 `oiG01` | 2 | 359 | 32 ms |
| Lichess mate-in-3 `UjZa9` | 3 | 31,804 | 1,882 ms |

## Iterative deepening

The public `solve` entry point searches mate-in-1 through the requested
maximum and returns the shortest proven mate.

| Fixture | Maximum depth | Nodes | Time |
|---|---:|---:|---:|
| Lichess mate-in-1 `yxtqU` | 1 | 24 | 1 ms |
| Lichess mate-in-2 `oiG01` | 2 | 393 | 33 ms |
| Lichess mate-in-3 `UjZa9` | 3 | 33,251 | 2,108 ms |

## Move ordering

Checks, captures, and killer moves are now searched first. Ordering does not
prune any legal move.

| Fixture | Maximum depth | Nodes | Time |
|---|---:|---:|---:|
| Lichess mate-in-1 `yxtqU` | 1 | 4 | 0 ms |
| Lichess mate-in-2 `oiG01` | 2 | 39 | 4 ms |
| Lichess mate-in-3 `UjZa9` | 3 | 502 | 32 ms |

## Transposition table

The solver uses a Zobrist-style position key containing the board, side to
move, castling rights, en passant square, and remaining ply. The table is
capped at 64 MiB.

| Fixture | Maximum depth | Nodes | Time |
|---|---:|---:|---:|
| Lichess mate-in-1 `yxtqU` | 1 | 4 | 0 ms |
| Lichess mate-in-2 `oiG01` | 2 | 39 | 2 ms |
| Lichess mate-in-3 `UjZa9` | 3 | 502 | 34 ms |
