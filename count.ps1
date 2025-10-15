[int]$totalLines = 0
$directory = Get-Location

$files = Get-ChildItem -Path $directory | Where-Object { -not $_.PSIsContainer -and ($_.Name -match '\.(cpp|hpp)$') }

foreach ($file in $files) {
    try {
        $lineCount = (Get-Content -LiteralPath $file.FullName | Measure-Object -Line).Lines
    } catch {
        Write-Host ("Error reading {0}: {1}" -f $file.Name, $_.Exception.Message)
        continue
    }
	
    Write-Host ("{0}: {1}" -f $file.Name, $lineCount)
    $totalLines += $lineCount
}

Write-Host ("Total: {0}" -f $totalLines)