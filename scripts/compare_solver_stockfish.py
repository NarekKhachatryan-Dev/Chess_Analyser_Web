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
    parser.add_argument(
        "--negative",
        action="store_true",
        help="check each fixture at one move shorter, skipping mate-in-1",
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
            requested_mate = mate_in - 1 if args.negative else mate_in
            if requested_mate < 1:
                continue
            cli = subprocess.run(
                [args.chess_cli, "solve", fen, str(requested_mate)],
                capture_output=True,
                text=True,
                check=False,
            )
            result = re.search(r"^(MateFound|NoMateWithinN|NoLegalMoves)", cli.stdout)
            solver_status = result.group(1) if result else None
            solver_mate = (
                int(re.search(r"length=(\d+)", cli.stdout).group(1))
                if solver_status == "MateFound"
                else None
            )

            stockfish.stdin.write(f"position fen {fen}\n")
            stockfish.stdin.flush()
            stockfish_mate, stockfish_move = read_stockfish(stockfish, requested_mate)
            if args.negative:
                agrees = (
                    cli.returncode == 0
                    and solver_status == "NoMateWithinN"
                    and stockfish_mate is None
                )
                description = f"no mate at N={requested_mate}"
            else:
                agrees = (
                    cli.returncode == 0
                    and solver_status == "MateFound"
                    and solver_mate == stockfish_mate
                )
                description = f"mate in {requested_mate}"
            if agrees:
                print(f"{puzzle_id}: {description}")
            else:
                disagreements.append(
                    f"{puzzle_id}: solver={cli.stdout.strip()!r}, "
                    f"stockfish_mate={stockfish_mate}, stockfish_bestmove={stockfish_move}"
                )

        stockfish.stdin.write("quit\n")
        stockfish.stdin.flush()

    print(f"Compared {len(fixtures)} fixtures; disagreements: {len(disagreements)}")
    for disagreement in disagreements:
        print(f"DISAGREEMENT {disagreement}")
    return 1 if disagreements else 0


if __name__ == "__main__":
    raise SystemExit(main())
