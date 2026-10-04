import { applyUciLine } from "./solution.js";

const pieces = ["P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"];
const symbols = { P: "♙", N: "♘", B: "♗", R: "♖", Q: "♕", K: "♔",
  p: "♟", n: "♞", b: "♝", r: "♜", q: "♛", k: "♚", ".": "" };
const fenInput = document.querySelector("#fen");
const boardElement = document.querySelector("#board");
const paletteElement = document.querySelector("#palette");
const sideElement = document.querySelector("#side");
const castlingElement = document.querySelector("#castling");
const epElement = document.querySelector("#ep");
const validationElement = document.querySelector("#validation");
const resultElement = document.querySelector("#result");
const progressElement = document.querySelector("#progress");
const lineElement = document.querySelector("#line");
const solveButton = document.querySelector("#solve");
const validateButton = document.querySelector("#validate");
const cancelButton = document.querySelector("#cancel");
const previousButton = document.querySelector("#previous");
const nextButton = document.querySelector("#next");
let board = Array(64).fill(".");
let selectedPiece = "P";
let solution = [];
let solutionIndex = 0;
let initialBoard = [];
let worker;
let ready = false;
let pendingAction;
let readyTimer;

function parseFen(fen) {
  const fields = fen.trim().split(/\s+/);
  if (fields.length < 4) throw new Error("FEN requires at least four fields");
  const squares = [];
  for (const rank of fields[0].split("/")) {
    for (const character of rank) {
      if (/[1-8]/.test(character)) squares.push(...Array(Number(character)).fill("."));
      else squares.push(character);
    }
  }
  if (squares.length !== 64) throw new Error("FEN board must contain 64 squares");
  board = squares;
  sideElement.value = fields[1];
  castlingElement.value = fields[2];
  epElement.value = fields[3];
}

function boardFen() {
  const ranks = [];
  for (let rank = 0; rank < 8; rank += 1) {
    let text = "", empty = 0;
    for (let file = 0; file < 8; file += 1) {
      const piece = board[rank * 8 + file];
      if (piece === ".") empty += 1;
      else { if (empty) text += empty; empty = 0; text += piece; }
    }
    if (empty) text += empty;
    ranks.push(text);
  }
  return `${ranks.join("/")} ${sideElement.value} ${castlingElement.value || "-"} ${epElement.value || "-"} 0 1`;
}

function renderBoard() {
  boardElement.replaceChildren();
  board.forEach((piece, index) => {
    const square = document.createElement("button");
    square.className = `square ${(Math.floor(index / 8) + index) % 2 ? "dark" : "light"}`;
    square.textContent = symbols[piece];
    square.title = `${String.fromCharCode(97 + index % 8)}${8 - Math.floor(index / 8)}`;
    square.onclick = () => { board[index] = selectedPiece; fenInput.value = boardFen(); renderBoard(); };
    boardElement.append(square);
  });
}

function updateNavigation() {
  previousButton.disabled = solutionIndex === 0;
  nextButton.disabled = solutionIndex >= solution.length;
}

function showSolutionPosition() {
  board = applyUciLine(initialBoard, solution, solutionIndex);
  renderBoard();
}

function showLine() {
  lineElement.textContent = solution
    .map((move, index) => `${index + 1}. ${move}`)
    .join(" ");
}

function friendlyResult(result) {
  switch (result.status) {
    case "MateFound": return `Mate in ${result.length} found`;
    case "NoMateWithinN": return `No forced mate in ${document.querySelector("#depth").value} moves`;
    case "Cancelled": return "Cancelled";
    case "TimedOut": return "Timed out";
    case "NoLegalMoves": return "Side to move has no legal moves";
    default: return result.status;
  }
}

pieces.forEach((piece) => {
  const button = document.createElement("button");
  button.textContent = symbols[piece];
  button.title = `Place ${piece}`;
  button.onclick = () => { selectedPiece = piece; };
  paletteElement.append(button);
});

function dispatchPending() {
  if (!ready || !pendingAction) return;
  const action = pendingAction;
  pendingAction = undefined;
  worker.postMessage(action);
}

function createWorker() {
  worker?.terminate();
  ready = false;
  clearTimeout(readyTimer);
  progressElement.textContent = "loading engine...";
  worker = new Worker("./worker.js", { type: "module" });
  worker.onmessage = ({ data }) => {
    if (data.type === "ready") {
      ready = true;
      clearTimeout(readyTimer);
      progressElement.textContent = "";
      dispatchPending();
      return;
    }
    if (data.type === "error") {
      progressElement.textContent = data.error;
      resultElement.textContent = data.error;
      cancelButton.disabled = true;
      return;
    }
    if (data.type === "validation") {
      validationElement.textContent = data.result.valid ? "Valid position" : data.result.errors.join("\n");
      validationElement.className = data.result.valid ? "ok" : "error";
      return;
    }
    if (data.type === "progress") {
      progressElement.textContent = `searching N=${data.depth}`;
      return;
    }
    if (data.type !== "result") return;
    progressElement.textContent = "";
    resultElement.textContent = friendlyResult(data.result);
    solution = data.result.line || [];
    solutionIndex = 0;
    showSolutionPosition();
    showLine();
    cancelButton.disabled = true;
    updateNavigation();
  };
  worker.onerror = (event) => {
    const message = event.message || "worker error";
    progressElement.textContent = message;
    resultElement.textContent = message;
    cancelButton.disabled = true;
  };
  worker.onmessageerror = () => {
    progressElement.textContent = "worker message error";
    resultElement.textContent = "worker message error";
    cancelButton.disabled = true;
  };
  readyTimer = setTimeout(() => {
    if (!ready) {
      progressElement.textContent = "engine did not start";
      resultElement.textContent = "engine did not start";
      solveButton.disabled = false;
      validateButton.disabled = false;
    }
  }, 10000);
  worker.postMessage({ type: "init" });
}

function queue(action) {
  pendingAction = action;
  dispatchPending();
}

validateButton.onclick = () => {
  try { parseFen(fenInput.value); } catch (error) {
    validationElement.textContent = error.message;
    validationElement.className = "error";
    return;
  }
  queue({ type: "validate", fen: boardFen() });
};
solveButton.onclick = () => {
  try { parseFen(fenInput.value); } catch (error) {
    resultElement.textContent = error.message;
    resultElement.className = "error";
    return;
  }
  resultElement.className = "";
  initialBoard = board.slice();
  solution = [];
  solutionIndex = 0;
  updateNavigation();
  progressElement.textContent = ready ? "starting" : "loading engine...";
  cancelButton.disabled = false;
  queue({ type: "solve", fen: boardFen(), maxDepth: Number(document.querySelector("#depth").value) });
};
cancelButton.onclick = () => {
  pendingAction = undefined;
  createWorker();
  cancelButton.disabled = true;
  progressElement.textContent = "";
  resultElement.textContent = "Cancelled";
};
previousButton.onclick = () => {
  solutionIndex = Math.max(0, solutionIndex - 1);
  showSolutionPosition();
  updateNavigation();
};
nextButton.onclick = () => {
  solutionIndex = Math.min(solution.length, solutionIndex + 1);
  showSolutionPosition();
  updateNavigation();
};
fenInput.onchange = () => {
  try { parseFen(fenInput.value); renderBoard(); } catch (error) {
    validationElement.textContent = error.message;
    validationElement.className = "error";
  }
};
parseFen(fenInput.value);
renderBoard();
updateNavigation();
createWorker();
