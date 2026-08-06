param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectDir
)

# MSBuild may pass the directory with trailing quote/backslash artifacts from command-line escaping
$ProjectDir = $ProjectDir.Trim().Trim('"').TrimEnd('\')

$buildNumberFile = Join-Path $ProjectDir "build_number.txt"
$versionBuildHeader = Join-Path $ProjectDir "src\version_build.h"

# Read current build number (default 0 if file missing)
$buildNumber = 0
if (Test-Path $buildNumberFile) {
    $content = (Get-Content $buildNumberFile -Raw).Trim()
    if ($content -match '^\d+$') {
        $buildNumber = [int]$content
    }
}

# Increment
$buildNumber++

# Write back
Set-Content -Path $buildNumberFile -Value $buildNumber -NoNewline

# Read version.h for MAJOR/MINOR/PATCH
$versionH = Join-Path $ProjectDir "src\version.h"
$major = 1
$minor = 0
$patch = 0
if (Test-Path $versionH) {
    $vh = Get-Content $versionH -Raw
    if ($vh -match '#define\s+RB_VERSION_MAJOR\s+(\d+)') { $major = $matches[1] }
    if ($vh -match '#define\s+RB_VERSION_MINOR\s+(\d+)') { $minor = $matches[1] }
    if ($vh -match '#define\s+RB_VERSION_PATCH\s+(\d+)') { $patch = $matches[1] }
}

$versionString = "$major.$minor.$patch.$buildNumber"

# Generate version_build.h (include guard for rc.exe compatibility)
$header = @"
#ifndef RENAME_BUILDINGS_VERSION_BUILD_H
#define RENAME_BUILDINGS_VERSION_BUILD_H

#define RB_VERSION_BUILD $buildNumber // NOLINT(modernize-macro-to-enum)
#define RB_VERSION_STRING "$versionString"

#endif // RENAME_BUILDINGS_VERSION_BUILD_H
"@

Set-Content -Path $versionBuildHeader -Value $header
