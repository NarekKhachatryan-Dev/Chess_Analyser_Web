# Testing

## Perft correctness gate

All values below are exact node counts and are registered as CTest tests.

| Position | d1 | d2 | d3 | d4 | d5 |
|---|---:|---:|---:|---:|---:|
| Start position | 20 | 400 | 8,902 | 197,281 | 4,865,609 |
| Kiwipete | 48 | 2,039 | 97,862 | 4,085,603 | - |
| Position 3 | 14 | 191 | 2,812 | 43,238 | 674,624 |
| Position 4 | 6 | 264 | 9,467 | 422,333 | - |
| Position 4 color mirror | 6 | 264 | 9,467 | 422,333 | - |
| Position 5 | 44 | 1,486 | 62,379 | 2,103,487 | - |

## Test groups

The final native CTest run contains 24 tests:

- 1 skeleton test.
- 5 solver fixture and solver-behavior tests, including stalemate avoidance.
- 4 solver edge-case tests: stalemate, no legal moves, cancellation/timeout,
  and no mate within N.
- 6 perft tests.
- 7 validation and rules tests.
- 1 additional checkmate/stalemate rules test.

The JavaScript test groups contain 5 tests for editor state and move
application. The WASM Node test contains 2 tests for perft and mate-in-2.

## Stockfish differential testing

The comparator starts Stockfish, sends `position fen ...`, then `go mate N`,
and compares the reported mate length with the CLI result. The positive corpus
contains 85 real Lichess fixtures:

- 30 mate-in-1
- 30 mate-in-2
- 20 mate-in-3
- 5 mate-in-4

The supplied `stalemate-avoidance` position is a separate fixture and is also
compared with Stockfish.

| Run | Compared | Disagreements |
|---|---:|---:|
| Lichess positive corpus | 85 | 0 |
| Stalemate-avoidance positive fixture | 1 | 0 |
| Combined positive result | 86 | 0 |
| Negative, eligible corpus | 55 | 0 |

Mate-in-1 cannot be tested at N-1 because that would require N=0. Negative
mode therefore uses the 55 mate-in-2-or-more rows. Stockfish `go mate N-1`
may report a longer mate, such as `mate 2` when asked for `mate 1`; that does
not contradict `NoMateWithinN`. A negative disagreement is counted only when
Stockfish reports a mate in at most N-1 moves. In the measured run every
reported Stockfish mate length M equaled the fixture's original N.

## Historical castling-rights bug

This was the step-by-step investigation:

1. **Measure the failing perft gates.** Before the fix, Kiwipete produced
   `97,655` instead of `97,862` at depth 3 and `4,058,106` instead of
   `4,085,603` at depth 4. Position 4 produced `421,897` instead of
   `422,333` at depth 4. All start-position numbers were correct.
2. **Compare root divides.** Comparing `divide` with Stockfish `go perft 3`
   on Kiwipete found the first differing root move:

   ```text
   a1b1: ours 1928, Stockfish 1969
   ```

3. **Recurse into the first difference.** Using
   `position fen <FEN> moves a1b1 e8g8` isolated this position:

   ```text
   r4rk1/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/1R2K2R w K - 2 2
   ```

   Stockfish's legal moves included `e1g1` (castling); our engine omitted it,
   with no extra moves.
4. **Find the cause.** Castling-right updates for rook movement and rook
   capture were reversed: a1 was treated as the king-side rook, h1 as the
   queen-side rook, and the same inversion occurred for a8/h8.
5. **Fix and verify.** The position code was corrected so each rook corner
   updates its corresponding castling right. After the fix, all five perft
   gates matched.
