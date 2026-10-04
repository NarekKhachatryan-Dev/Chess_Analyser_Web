import test from "node:test";
import assert from "node:assert/strict";
import { pathToFileURL } from "node:url";

const moduleUrl = process.env.CHESS_WASM_MODULE;
assert.ok(moduleUrl, "CHESS_WASM_MODULE must point to the Emscripten module");
const createEngine = (await import(pathToFileURL(moduleUrl))).default;
const engine = await createEngine();
const perft = engine.cwrap("perft", "number", ["string", "number"]);
const solve = engine.cwrap("solve_depth", "number", ["string", "number"]);
const free = engine.cwrap("free_json", null, ["number"]);

function call(pointer) {
  const value = engine.UTF8ToString(pointer);
  free(pointer);
  return JSON.parse(value);
}

test("WASM perft matches the start position", () => {
  const result = call(perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2));
  assert.equal(result.nodes, 400);
});

test("WASM solves a real mate-in-2", () => {
  const result = call(solve("2kr4/2p3p1/2P1b1Np/1rn5/8/5P2/1B4PP/R5K1 w - - 0 28", 2));
  assert.equal(result.status, "MateFound");
  assert.equal(result.length, 2);
  assert.equal(result.line[0], "a1a8");
});
