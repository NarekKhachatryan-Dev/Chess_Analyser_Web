export function normalizeCastling(value) {
  const text = value.trim();
  if (text === "-") return "-";
  if (!text || text.includes("-") || !/^[KQkq]+$/.test(text) || new Set(text).size !== text.length) {
    throw new Error("Castling rights must be -, or unique letters K, Q, k, q");
  }
  return ["K", "Q", "k", "q"].filter((right) => text.includes(right)).join("");
}

export function validMateDepth(value, max = 5) {
  return Number.isInteger(Number(value)) && Number(value) >= 1 && Number(value) <= max;
}
