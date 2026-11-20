$ErrorActionPreference = 'Stop'
$fail = $false

function Fail($msg) {
    Write-Host "::error::$msg"
    $script:fail = $true
}

$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$sourceFiles = Get-ChildItem -Path $root -Recurse -File -Include *.cpp,*.h,*.rc,*.vcxproj,*.props | Where-Object { $_.FullName -notmatch '\\(bin|x64|\.git|scripts|Wireless\.[^\\]+)\\' }

# 1. No absolute developer paths or debug log side effects in shipped sources
$patterns = @(
    @{ Name = 'absolute drive path'; Regex = '[A-Za-z]:\\(Temp|repos|Users)' },
    @{ Name = 'debug log output'; Regex = 'wofstream|OutputDebugString|wb_[a-z_]+\.log' },
    @{ Name = 'TODO marker'; Regex = 'TODO:' },
    @{ Name = 'absolute import lib'; Regex = '(?i)#pragma\s+comment\(lib,\s*"[A-Za-z]:\\' }
)
foreach ($file in $sourceFiles) {
    $content = Get-Content $file.FullName -Raw -ErrorAction SilentlyContinue
    if (-not $content) { continue }
    foreach ($p in $patterns) {
        if ($content -match $p.Regex) {
            Fail "$($p.Name) in $($file.FullName.Substring($root.Length + 1))"
        }
    }
}

# 2. Required release assets exist
foreach ($required in @('LICENSE', 'THIRD_PARTY_NOTICES.md', 'THIRD_PARTY_RUST_LICENSES.txt', 'README.md', 'lib\x64\wireless_battery.lib')) {
    if (-not (Test-Path (Join-Path $root $required))) {
        Fail "missing required file: $required"
    }
}

if ($fail) { exit 1 }
Write-Host 'static checks passed'