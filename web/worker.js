import createChessEngine from "./chess_engine.js";

const enginePromise = createChessEngine();
let engine;
let api;
let readySent = false;
let loading;

function jsonCall(pointer) {
  const value = engine.UTF8ToString(pointer);
  api.freeJson(pointer);
  return JSON.parse(value);
}

async function loadEngine() {
  engine = await enginePromise;
  api = {
    solveDepth: engine.cwrap("solve_depth", "number", ["string", "number"]),
    validate: engine.cwrap("validate", "number", ["string"]),
    perft: engine.cwrap("perft", "number", ["string", "number"]),
    freeJson: engine.cwrap("free_json", null, ["number"]),
  };
  if (!readySent) {
    readySent = true;
    self.postMessage({ type: "ready" });
  }
}

self.onmessage = async ({ data }) => {
  try {
    if (!loading) {
      loading = loadEngine();
    }
    await loading;
    if (data.type === "init") return;
    if (data.type === "validate") {
      self.postMessage({ type: "validation", result: jsonCall(api.validate(data.fen)) });
      return;
    }
    if (data.type === "perft") {
      self.postMessage({ type: "perft", result: jsonCall(api.perft(data.fen, data.depth)) });
      return;
    }
    if (data.type !== "solve") return;
    for (let depth = 1; depth <= data.maxDepth; depth += 1) {
      self.postMessage({ type: "progress", depth });
      const result = jsonCall(api.solveDepth(data.fen, depth));
      if (result.status === "MateFound" || result.status === "NoLegalMoves" ||
          result.status === "Cancelled" || result.status === "TimedOut") {
        self.postMessage({ type: "result", result });
        return;
      }
    }
    self.postMessage({ type: "result", result: { status: "NoMateWithinN", line: [], length: 0 } });
  } catch (error) {
    self.postMessage({ type: "error", error: error instanceof Error ? error.message : String(error) });
  }
};
