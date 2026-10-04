import test from "node:test";
import assert from "node:assert/strict";
import { applyUciMove } from "../web/solution.js";

const emptyBoard = () => Array(64).fill(".");
const index = (square) => (8 - Number(square[1])) * 8 + square.charCodeAt(0) - 97;

test("white promotion uses an uppercase piece", () => {
  const board = emptyBoard();
  board[index("a7")] = "P";
  assert.equal(applyUciMove(board, "a7a8q")[index("a8")], "Q");
});

test("castling moves the rook", () => {
  const board = emptyBoard();
  board[index("e1")] = "K";
  board[index("h1")] = "R";
  const result = applyUciMove(board, "e1g1");
  assert.equal(result[index("g1")], "K");
  assert.equal(result[index("f1")], "R");
  assert.equal(result[index("h1")], ".");
});

test("en passant removes the captured pawn", () => {
  const board = emptyBoard();
  board[index("e5")] = "P";
  board[index("d5")] = "p";
  const result = applyUciMove(board, "e5d6");
  assert.equal(result[index("d6")], "P");
  assert.equal(result[index("d5")], ".");
});
