$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $projectRoot

$wasmJavaScript = Join-Path $projectRoot "web\chess_engine.js"
$wasmBinary = Join-Path $projectRoot "web\chess_engine.wasm"

if (-not (Test-Path $wasmJavaScript) -or -not (Test-Path $wasmBinary)) {
    if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
        $emsdkEnv = Join-Path $env:USERPROFILE "emsdk\emsdk_env.ps1"
        if (-not (Test-Path $emsdkEnv)) {
            Write-Error "Emscripten was not found. Install emsdk at '$($env:USERPROFILE)\emsdk' or put emcc on PATH."
            exit 1
        }
        Write-Host "emcc is not on PATH; activating $emsdkEnv"
        & $emsdkEnv
    }

    if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
        Write-Error "Emscripten activation completed, but emcc is still unavailable."
        exit 1
    }

    Write-Host "WASM files are missing; building the web engine..."
    & (Join-Path $PSScriptRoot "build_wasm.ps1")
}

if (-not (Test-Path $wasmJavaScript) -or -not (Test-Path $wasmBinary)) {
    Write-Error "WASM build did not produce web/chess_engine.js and web/chess_engine.wasm."
    exit 1
}

Write-Host "Opening http://127.0.0.1:8000/"
Start-Process "http://127.0.0.1:8000/"
Write-Host "Starting the web server. Stop it with Ctrl+C."
python -m http.server 8000 --directory web
