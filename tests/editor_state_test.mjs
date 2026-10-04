import test from "node:test";
import assert from "node:assert/strict";
import { normalizeCastling, validMateDepth } from "../web/editor_state.js";

test("editor normalizes and rejects castling rights", () => {
  assert.equal(normalizeCastling("kqKQ"), "KQkq");
  assert.throws(() => normalizeCastling("KQK"), /unique letters/);
  assert.throws(() => normalizeCastling("KX"), /unique letters/);
  assert.throws(() => normalizeCastling("K-"), /unique letters/);
});

test("editor accepts only mate depths from one through five", () => {
  assert.equal(validMateDepth("1"), true);
  assert.equal(validMateDepth("5"), true);
  assert.equal(validMateDepth("0"), false);
  assert.equal(validMateDepth("6"), false);
  assert.equal(validMateDepth("50"), false);
  assert.equal(validMateDepth("2.5"), false);
});
