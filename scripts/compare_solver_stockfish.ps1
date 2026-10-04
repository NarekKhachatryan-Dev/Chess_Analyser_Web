param(
    [string]$ChessCli = ".\build\cli\chess_cli.exe",
    [string]$Stockfish = "stockfish.exe"
)

$fixtures = @(
    @{ Id = "yxtqU"; Fen = "5k2/r3qp2/6p1/1P6/8/1BQ1P3/2P2P2/2K5 w - - 4 41"; N = 1 },
    @{ Id = "oiG01"; Fen = "2kr4/2p3p1/2P1b1Np/1rn5/8/5P2/1B4PP/R5K1 w - - 0 28"; N = 2 },
    @{ Id = "UjZa9"; Fen = "6rk/p6p/1p2Qpr1/8/3PRP2/q5P1/7P/4R1K1 b - - 0 27"; N = 3 }
)

function Invoke-StockfishMate([string]$Fen, [int]$N) {
    $info = New-Object System.Diagnostics.ProcessStartInfo
    $info.FileName = $Stockfish
    $info.UseShellExecute = $false
    $info.RedirectStandardInput = $true
    $info.RedirectStandardOutput = $true
    $info.CreateNoWindow = $true
    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $info
    if (-not $process.Start()) {
        throw "Could not start Stockfish: $Stockfish"
    }
    $process.StandardInput.WriteLine("uci")
    while (-not $process.StandardOutput.EndOfStream) {
        $line = $process.StandardOutput.ReadLine()
        if ($line -eq "uciok") {
            break
        }
    }
    $process.StandardInput.WriteLine("isready")
    while (-not $process.StandardOutput.EndOfStream) {
        $line = $process.StandardOutput.ReadLine()
        if ($line -eq "readyok") {
            break
        }
    }
    $process.StandardInput.WriteLine("position fen $Fen")
    $process.StandardInput.WriteLine("go mate $N")
    $mate = $null
    $bestMove = $null
    while (-not $process.StandardOutput.EndOfStream) {
        $line = $process.StandardOutput.ReadLine()
        Write-Verbose "Stockfish: $line"
        $scoreMatch = [regex]::Match($line, "score mate (-?\d+)")
        if ($scoreMatch.Success) {
            $mate = [int]$scoreMatch.Groups[1].Value
        }
        $bestMoveMatch = [regex]::Match($line, "^bestmove\s+(\S+)")
        if ($bestMoveMatch.Success) {
            $bestMove = $bestMoveMatch.Groups[1].Value
            break
        }
    }
    $process.StandardInput.WriteLine("quit")
    $process.WaitForExit(5000) | Out-Null
    $script:StockfishMate = $mate
    $script:StockfishBestMove = $bestMove
}

foreach ($fixture in $fixtures) {
    $output = & $ChessCli solve $fixture.Fen $fixture.N
    if ($LASTEXITCODE -ne 0 -or $output -notmatch "^MateFound length=(\d+)") {
        throw "ChessSolver did not find mate for $($fixture.Id): $output"
    }
    $ourLength = [int]$Matches[1]
    Invoke-StockfishMate $fixture.Fen $fixture.N | Out-Null
    $stockfishMate = $script:StockfishMate
    if ($null -eq $stockfishMate -or ([int]$stockfishMate -ne [int]$ourLength)) {
        throw "Mismatch for $($fixture.Id): ChessSolver mate $ourLength, Stockfish mate $stockfishMate"
    }
    Write-Output "$($fixture.Id): ChessSolver mate $ourLength; Stockfish mate $stockfishMate, bestmove $script:StockfishBestMove"
}
