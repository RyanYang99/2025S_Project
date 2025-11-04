param(
    [switch]$ShowCleanFiles = $false
)

$ScriptDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

$CFiles = Get-ChildItem -Path $ScriptDirectory -Filter "*.cpp" -File
$HFiles = Get-ChildItem -Path $ScriptDirectory -Filter "*.hpp" -File
$AllFiles = $CFiles + $HFiles

if ($AllFiles.Count -eq 0) {
    exit
}

$FilesWithTrailingSpaces = 0
$TotalLinesWithTrailingSpaces = 0

foreach ($File in $AllFiles) {
    $Content = Get-Content -Path $File.FullName
    $LinesWithTrailingSpaces = @()
    
    for ($i = 0; $i -lt $Content.Count; $i++) {
        $Line = $Content[$i]

        if ($Line -match '\s+$') {
            $LinesWithTrailingSpaces += ($i + 1)
        }
    }
    
    if ($LinesWithTrailingSpaces.Count -gt 0) {
        $FilesWithTrailingSpaces++
        $TotalLinesWithTrailingSpaces += $LinesWithTrailingSpaces.Count
        
        Write-Host "FILE: $($File.Name)" -ForegroundColor Red
        Write-Host "  Lines with trailing spaces: $($LinesWithTrailingSpaces -join ', ')" -ForegroundColor Yellow
        Write-Host "  Total lines affected: $($LinesWithTrailingSpaces.Count)" -ForegroundColor Yellow
        Write-Host ""
    } elseif ($ShowCleanFiles) {
        Write-Host "FILE: $($File.Name)" -ForegroundColor Green
        Write-Host "  No trailing spaces found" -ForegroundColor Green
        Write-Host ""
    }
}

Write-Host "SUMMARY:" -ForegroundColor Cyan
Write-Host "Files checked: $($AllFiles.Count)" -ForegroundColor White
Write-Host "Files with trailing spaces: $FilesWithTrailingSpaces" -ForegroundColor $(if ($FilesWithTrailingSpaces -gt 0) { "Red" } else { "Green" })
Write-Host "Total lines with trailing spaces: $TotalLinesWithTrailingSpaces" -ForegroundColor $(if ($TotalLinesWithTrailingSpaces -gt 0) { "Red" } else { "Green" })