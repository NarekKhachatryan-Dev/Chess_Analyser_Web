# Limitations and future work

## Current limitations

- Only mate-in-N for the side to move is implemented.
- Draw rules, repetition, the fifty-move rule, and insufficient-material
  adjudication are not implemented.
- N is limited to 5 by the C wrapper and web UI.
- The default C-wrapper time limit is 30 seconds.
- The solver is single-threaded.
- The fixture corpus consists of Lichess mate puzzles and the supplied
  stalemate-avoidance fixture; it is not a general chess test corpus.
- WASM uses fixed initial memory and a fixed transposition-table budget; larger
  positions may time out or exceed those limits.
- SAN generation is not implemented; displayed solution moves remain UCI.
- Browser verification was performed with Chromium only. Firefox, Safari, and
  mobile browsers were not verified.
- There is no desktop GUI, play mode, evaluation, or multi-variation analysis.

## Future work

- Add draw and repetition handling.
- Add SAN generation and richer notation.
- Add worker-side progress metrics and optional principal variations.
- Add controlled parallel search for native builds.
- Measure and support additional browsers and mobile layouts.
- Add a larger independent test corpus and reproducible benchmark harnesses.
