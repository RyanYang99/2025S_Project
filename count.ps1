# CountLines.ps1
# PowerShell v2.0 compatible
# Counts lines in .h and .c files in the current directory (non-recursive)

[int]$totalLines = 0
$directory = Get-Location

# Get items in the current directory, filter to files whose names end with .c or .h (case-insensitive)
$files = Get-ChildItem -Path $directory | Where-Object { -not $_.PSIsContainer -and ($_.Name -match '\.(c|h)$') }

if (-not $files) {
    Write-Host "No .c or .h files found in $directory"
    exit 0
}

foreach ($file in $files) {
    try {
        $lineCount = (Get-Content -LiteralPath $file.FullName | Measure-Object -Line).Lines
    } catch {
        Write-Host ("Error reading {0}: {1}" -f $file.Name, $_.Exception.Message)
        continue
    }
    Write-Host ("{0} : {1}" -f $file.Name, $lineCount)
    $totalLines += $lineCount
}

Write-Host "-----------------------------"
Write-Host ("Total lines in .h and .c files: {0}" -f $totalLines)
