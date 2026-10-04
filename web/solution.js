export function applyUciMove(board, uci) {
  const next = board.slice();
  const from = (8 - Number(uci[1])) * 8 + uci.charCodeAt(0) - 97;
  const to = (8 - Number(uci[3])) * 8 + uci.charCodeAt(2) - 97;
  const piece = next[from];
  const target = next[to];

  if ((piece === "K" || piece === "k") && Math.abs((to % 8) - (from % 8)) === 2) {
    const rookFrom = to > from ? Math.floor(from / 8) * 8 + 7 : Math.floor(from / 8) * 8;
    const rookTo = to > from ? to - 1 : to + 1;
    next[rookTo] = next[rookFrom];
    next[rookFrom] = ".";
  }
  if ((piece === "P" || piece === "p") && target === "." && from % 8 !== to % 8) {
    const capturedPawn = to + (piece === "P" ? 8 : -8);
    next[capturedPawn] = ".";
  }
  next[to] = uci.length === 5
    ? (piece === piece.toUpperCase() ? uci[4].toUpperCase() : uci[4])
    : piece;
  next[from] = ".";
  return next;
}

export function applyUciLine(board, line, count = line.length) {
  return line.slice(0, count).reduce(applyUciMove, board);
}
