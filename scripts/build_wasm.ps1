param(
    [string]$BuildDirectory = "build_wasm"
)

emcmake cmake -S . -B $BuildDirectory -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build $BuildDirectory --target chess_engine_wasm
Copy-Item "$BuildDirectory\engine\chess_engine.js" "web\chess_engine.js" -Force
Copy-Item "$BuildDirectory\engine\chess_engine.wasm" "web\chess_engine.wasm" -Force
