import { applyUciLine } from "./solution.js";
import { normalizeCastling, validMateDepth } from "./editor_state.js";

const pieces = ["P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"];
const symbols = { P: "♙", N: "♘", B: "♗", R: "♖", Q: "♕", K: "♔",
  p: "♟", n: "♞", b: "♝", r: "♜", q: "♛", k: "♚", ".": "" };
const imageNames = { P: "w_p", N: "w_n", B: "w_b", R: "w_r", Q: "w_q", K: "w_k",
  p: "b_p", n: "b_n", b: "b_b", r: "b_r", q: "b_q", k: "b_k" };
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
const clearBoardButton = document.querySelector("#clear-board");
const startPositionButton = document.querySelector("#start-position");
const flipBoardButton = document.querySelector("#flip-board");
const filesElement = document.querySelector("#files");
const ranksElement = document.querySelector("#ranks");
const depthInput = document.querySelector("#depth");
const depthError = document.querySelector("#depth-error");
const moveCount = document.querySelector("#move-count");
const startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
let board = Array(64).fill(".");
let selectedPiece = ".";
let solution = [];
let solutionIndex = 0;
let initialBoard = [];
let worker;
let ready = false;
let pendingAction;
let readyTimer;
let boardFlipped = false;
let initialSide = "w";

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
  const indices = Array.from({ length: 64 }, (_, index) => boardFlipped ? 63 - index : index);
  indices.forEach((index) => {
    const square = document.createElement("button");
    const piece = board[index];
    square.className = `square ${(Math.floor(index / 8) + index) % 2 ? "dark" : "light"}`;
    square.dataset.square = index;
    if (imageNames[piece]) {
      const image = document.createElement("img");
      image.src = `assets/${imageNames[piece]}.png`;
      image.alt = symbols[piece];
      square.append(image);
    }
    square.title = `${String.fromCharCode(97 + index % 8)}${8 - Math.floor(index / 8)}`;
    square.onclick = () => {
      board[index] = selectedPiece;
      fenInput.value = boardFen();
      clearPositionMessages();
      clearAnalysis();
      renderBoard();
    };
    boardElement.append(square);
  });
  if (solutionIndex > 0 && solutionIndex <= solution.length) {
    const move = solution[solutionIndex - 1];
    const from = (move.charCodeAt(0) - 97) + (Number(move[1]) - 1) * 8;
    const to = (move.charCodeAt(2) - 97) + (Number(move[3]) - 1) * 8;
    boardElement.querySelector(`[data-square="${from}"]`)?.classList.add("last-from");
    boardElement.querySelector(`[data-square="${to}"]`)?.classList.add("last-to");
  }
  filesElement.replaceChildren(...(boardFlipped ? [..."hgfedcba"] : [..."abcdefgh"]).map((file) => {
    const label = document.createElement("span");
    label.textContent = file;
    return label;
  }));
  ranksElement.replaceChildren(...(boardFlipped ? ["1", "2", "3", "4", "5", "6", "7", "8"] :
    ["8", "7", "6", "5", "4", "3", "2", "1"]).map((rank) => {
    const label = document.createElement("span");
    label.textContent = rank;
    return label;
  }));
}

function clearPositionMessages() {
  validationElement.textContent = "";
  validationElement.className = "";
}

function clearAnalysis() {
  resultElement.textContent = "";
  resultElement.className = "";
  progressElement.textContent = "";
  lineElement.textContent = "";
  solution = [];
  solutionIndex = 0;
  depthError.textContent = "";
  updateNavigation();
}

function loadPosition(fen) {
  parseFen(fen);
  fenInput.value = fen;
  clearPositionMessages();
  clearAnalysis();
  renderBoard();
}

function currentEditorFen() {
  try {
    castlingElement.value = normalizeCastling(castlingElement.value);
    validationElement.textContent = "";
    validationElement.className = "";
    return boardFen();
  } catch (error) {
    validationElement.textContent = error.message;
    validationElement.className = "error";
    return undefined;
  }
}

function updateNavigation() {
  previousButton.disabled = solutionIndex === 0;
  nextButton.disabled = solutionIndex >= solution.length;
  moveCount.textContent = `Step ${solutionIndex} of ${solution.length}`;
}

function showSolutionPosition() {
  board = applyUciLine(initialBoard, solution, solutionIndex);
  renderBoard();
}

function showLine() {
  const parts = [];
  let moveNumber = 1;
  let index = 0;
  if (initialSide === "b" && solution.length > 0) {
    parts.push(`${moveNumber}... ${solution[index++]}`);
    moveNumber += 1;
  }
  while (index < solution.length) {
    parts.push(`${moveNumber}. ${solution[index++]}`);
    if (index < solution.length) parts.push(solution[index++]);
    moveNumber += 1;
  }
  lineElement.textContent = parts.join(" ");
}

function friendlyResult(result) {
  switch (result.status) {
    case "MateFound": return `Mate in ${result.length} found`;
    case "NoMateWithinN": return `No forced mate in ${document.querySelector("#depth").value} moves`;
    case "Cancelled": return "Cancelled";
    case "TimedOut": return "Time limit reached, no result";
    case "NoLegalMoves": return "Side to move has no legal moves";
    default: return result.status;
  }
}

function updatePaletteSelection() {
  [...paletteElement.children].forEach((button) => {
    button.classList.toggle("selected", button.dataset.piece === selectedPiece);
  });
}

pieces.forEach((piece) => {
  const button = document.createElement("button");
  const image = document.createElement("img");
  image.src = `assets/${imageNames[piece]}.png`;
  image.alt = symbols[piece];
  button.append(image);
  button.dataset.piece = piece;
  button.title = `Place ${piece}`;
  button.onclick = () => { selectedPiece = piece; updatePaletteSelection(); };
  paletteElement.append(button);
});
const eraserButton = document.createElement("button");
eraserButton.textContent = "Eraser";
eraserButton.dataset.piece = ".";
eraserButton.title = "Remove a piece";
eraserButton.onclick = () => { selectedPiece = "."; updatePaletteSelection(); };
paletteElement.append(eraserButton);

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
  const fen = currentEditorFen();
  if (fen) queue({ type: "validate", fen });
};
solveButton.onclick = () => {
  const depth = Number(depthInput.value);
  if (!validMateDepth(depth)) {
    depthError.textContent = "Mate in N must be a whole number from 1 to 5";
    return;
  }
  depthError.textContent = "";
  const fen = currentEditorFen();
  if (!fen) return;
  clearAnalysis();
  resultElement.className = "";
  initialBoard = board.slice();
  initialSide = sideElement.value;
  solution = [];
  solutionIndex = 0;
  updateNavigation();
  progressElement.textContent = ready ? "starting" : "loading engine...";
  cancelButton.disabled = false;
  queue({ type: "solve", fen, maxDepth: depth });
};
clearBoardButton.onclick = () => loadPosition("8/8/8/8/8/8/8/8 w - - 0 1");
startPositionButton.onclick = () => loadPosition(startFen);
flipBoardButton.onclick = () => { boardFlipped = !boardFlipped; renderBoard(); };
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
document.addEventListener("keydown", (event) => {
  if (event.target instanceof Element && event.target.matches("input, select, textarea")) return;
  if (event.key === "ArrowLeft" && !previousButton.disabled) previousButton.click();
  if (event.key === "ArrowRight" && !nextButton.disabled) nextButton.click();
});
fenInput.onchange = () => {
  try {
    parseFen(fenInput.value);
    castlingElement.value = normalizeCastling(castlingElement.value);
    fenInput.value = boardFen();
    clearPositionMessages();
    clearAnalysis();
    renderBoard();
  } catch (error) {
    validationElement.textContent = error.message;
    validationElement.className = "error";
  }
};
sideElement.onchange = () => { clearPositionMessages(); clearAnalysis(); fenInput.value = boardFen(); };
castlingElement.onchange = () => {
  try {
    castlingElement.value = normalizeCastling(castlingElement.value);
    clearPositionMessages();
    clearAnalysis();
    fenInput.value = boardFen();
  } catch (error) {
    validationElement.textContent = error.message;
    validationElement.className = "error";
  }
};
epElement.onchange = () => { clearPositionMessages(); clearAnalysis(); fenInput.value = boardFen(); };
parseFen(fenInput.value);
renderBoard();
updatePaletteSelection();
updateNavigation();
createWorker();
