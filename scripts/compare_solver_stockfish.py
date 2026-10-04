#!/usr/bin/env python3
"""Compare every real Lichess fixture in tests/data with Stockfish."""

from __future__ import annotations

import argparse
import re
import subprocess
from pathlib import Path


def read_fixtures(path: Path):
    for line in path.read_text(encoding="ascii").splitlines():
        if not line or line.startswith("#"):
            continue
        puzzle_id, mate_in, fen = line.split("\t", 2)
        yield puzzle_id, int(mate_in), fen


def read_stockfish(process: subprocess.Popen[str], mate_in: int) -> tuple[int | None, str | None]:
    process.stdin.write(f"go mate {mate_in}\n")
    process.stdin.flush()
    mate = None
    best_move = None
    while True:
        line = process.stdout.readline()
        if not line:
            raise RuntimeError("Stockfish exited before returning bestmove")
        match = re.search(r"score mate (-?\d+)", line)
        if match:
            mate = int(match.group(1))
        match = re.match(r"bestmove\s+(\S+)", line)
        if match:
            best_move = match.group(1)
            return mate, best_move


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--chess-cli", required=True)
    parser.add_argument("--stockfish", default="stockfish")
    parser.add_argument(
        "--data",
        default="tests/data/lichess_mate_puzzles.tsv",
    )
    args = parser.parse_args()

    fixtures = list(read_fixtures(Path(args.data)))
    disagreements: list[str] = []
    with subprocess.Popen(
        [args.stockfish],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    ) as stockfish:
        assert stockfish.stdin is not None
        assert stockfish.stdout is not None
        stockfish.stdin.write("uci\n")
        stockfish.stdin.flush()
        while stockfish.stdout.readline().strip() != "uciok":
            pass
        stockfish.stdin.write("isready\n")
        stockfish.stdin.flush()
        while stockfish.stdout.readline().strip() != "readyok":
            pass

        for puzzle_id, mate_in, fen in fixtures:
            cli = subprocess.run(
                [args.chess_cli, "solve", fen, str(mate_in)],
                capture_output=True,
                text=True,
                check=False,
            )
            result = re.search(r"^(MateFound|NoMateWithinN) length=(\d+)", cli.stdout)
            solver_mate = int(result.group(2)) if result and result.group(1) == "MateFound" else None

            stockfish.stdin.write(f"position fen {fen}\n")
            stockfish.stdin.flush()
            stockfish_mate, stockfish_move = read_stockfish(stockfish, mate_in)
            if cli.returncode != 0 or solver_mate != stockfish_mate:
                disagreements.append(
                    f"{puzzle_id}: solver={cli.stdout.strip()!r}, "
                    f"stockfish_mate={stockfish_mate}, stockfish_bestmove={stockfish_move}"
                )
            else:
                print(f"{puzzle_id}: mate {solver_mate}, Stockfish first move {stockfish_move}")

        stockfish.stdin.write("quit\n")
        stockfish.stdin.flush()

    print(f"Compared {len(fixtures)} fixtures; disagreements: {len(disagreements)}")
    for disagreement in disagreements:
        print(f"DISAGREEMENT {disagreement}")
    return 1 if disagreements else 0


if __name__ == "__main__":
    raise SystemExit(main())
