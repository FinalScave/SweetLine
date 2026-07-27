param(
    [Parameter(Mandatory = $true)]
    [Alias("v")]
    [string]$Version,

    [string]$PrebuiltSourceDir = "",
    [string]$HeadersSourceDir = "",

    [Alias("o")]
    [string]$OutputDir = "",

    [string]$Commit = "",

    [string]$ReleaseNotesTemplate = "",
    [switch]$SkipReleaseNotes,
    [switch]$Force
)

$ErrorActionPreference = "Stop"

if ($Version -notmatch '^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$') {
    throw "Invalid version: $Version"
}

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = [System.IO.Path]::GetFullPath((Join-Path $ScriptDir ".."))
$ResolvedPrebuiltSourceDir = if ($PrebuiltSourceDir) {
    [System.IO.Path]::GetFullPath($PrebuiltSourceDir)
} else {
    Join-Path $ProjectDir "prebuilt"
}
$ResolvedHeadersSourceDir = if ($HeadersSourceDir) {
    [System.IO.Path]::GetFullPath($HeadersSourceDir)
} else {
    Join-Path $ProjectDir "include"
}
$ResolvedReleaseNotesTemplate = if ($ReleaseNotesTemplate) {
    [System.IO.Path]::GetFullPath($ReleaseNotesTemplate)
} else {
    Join-Path $ScriptDir "RELEASE_NOTES.md"
}
$ResolvedOutputDir = if ($OutputDir) {
    [System.IO.Path]::GetFullPath($OutputDir)
} else {
    Join-Path $ProjectDir "build\artifacts"
}
$RequiredPlatforms = @("android", "ios", "linux", "macos", "ohos", "wasm", "windows")
$RequiredPrebuiltFiles = @(
    "android/arm64-v8a/libsweetline.so",
    "android/x86_64/libsweetline.so",
    "ios/arm64/libsweetline.dylib",
    "ios/simulator-arm64/libsweetline.dylib",
    "ios/SweetLineCoreIOS.xcframework.zip",
    "linux/aarch64/libsweetline.so",
    "linux/x86_64/libsweetline.so",
    "macos/arm64/libsweetline.dylib",
    "macos/x86_64/libsweetline.dylib",
    "macos/SweetLineCoreMacOS.xcframework.zip",
    "ohos/arm64-v8a/libsweetline.so",
    "ohos/x86_64/libsweetline.so",
    "wasm/sweetline.d.ts",
    "wasm/sweetline.js",
    "wasm/sweetline.wasm",
    "windows/x64/sweetline.dll"
)

function Ensure-Directory {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path | Out-Null
    }
}

function Remove-PathIfExists {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (Test-Path -LiteralPath $Path) {
        Remove-Item -LiteralPath $Path -Recurse -Force
    }
}

function Get-RelativePathNormalized {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$TargetPath
    )

    $baseFullPath = [System.IO.Path]::GetFullPath($BasePath).TrimEnd('\', '/')
    $targetFullPath = [System.IO.Path]::GetFullPath($TargetPath)

    $basePathWithSeparator = if ($baseFullPath.EndsWith('\') -or $baseFullPath.EndsWith('/')) {
        $baseFullPath
    } else {
        $baseFullPath + [System.IO.Path]::DirectorySeparatorChar
    }
    $baseUri = [System.Uri]$basePathWithSeparator
    $targetUri = [System.Uri]$targetFullPath
    $relativeUri = $baseUri.MakeRelativeUri($targetUri)
    return [System.Uri]::UnescapeDataString($relativeUri.ToString()) -replace '\\', '/'
}

function Copy-DirectoryContents {
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [Parameter(Mandatory = $true)][string]$Destination
    )

    Ensure-Directory -Path $Destination
    Copy-Item -Path (Join-Path $Source "*") -Destination $Destination -Recurse -Force
}

function Write-NativeReadmeFile {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$VersionText,
        [Parameter(Mandatory = $true)][string[]]$Platforms,
        [string]$CommitText = ""
    )

    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add("SweetLine Native SDK")
    $lines.Add("====================")
    $lines.Add("")
    $lines.Add("Version: $VersionText")
    if ($CommitText) {
        $lines.Add("Commit: $CommitText")
    }
    $lines.Add("Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss zzz')")
    $lines.Add("")
    $lines.Add("Package layout:")
    $lines.Add("- include/sweetline/: C/C++ headers")
    $lines.Add("- prebuilt/: native binaries grouped by platform")
    $lines.Add("")
    $lines.Add("Included prebuilt platform directories:")
    foreach ($platformName in $Platforms) {
        $lines.Add("- $platformName")
    }
    $lines.Add("")
    $lines.Add("Examples:")
    $lines.Add("- prebuilt/windows/x64/sweetline.dll")
    $lines.Add("- prebuilt/wasm/sweetline.js")
    $lines.Add("- prebuilt/android/arm64-v8a/libsweetline.so")

    [System.IO.File]::WriteAllLines($Path, $lines, [System.Text.UTF8Encoding]::new($false))
}

function Write-ChecksumsFile {
    param([Parameter(Mandatory = $true)][string]$StageDir)

    $checksumPath = Join-Path $StageDir "SHA256SUMS.txt"
    $lines = New-Object System.Collections.Generic.List[string]

    $files = Get-ChildItem -LiteralPath $StageDir -Recurse -File |
        Where-Object { $_.Name -notin @("SHA256SUMS.txt") } |
        Sort-Object FullName

    foreach ($file in $files) {
        $hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        $relativePath = Get-RelativePathNormalized -BasePath $StageDir -TargetPath $file.FullName
        $lines.Add("$hash  $relativePath")
    }

    [System.IO.File]::WriteAllLines($checksumPath, $lines, [System.Text.UTF8Encoding]::new($false))
}

function New-ZipFromDirectoryContents {
    param(
        [Parameter(Mandatory = $true)][string]$SourceDir,
        [Parameter(Mandatory = $true)][string]$ZipPath
    )

    if (Test-Path -LiteralPath $ZipPath) {
        Remove-Item -LiteralPath $ZipPath -Force
    }

    $zip = [System.IO.Compression.ZipFile]::Open($ZipPath, [System.IO.Compression.ZipArchiveMode]::Create)
    try {
        $sourceFullPath = [System.IO.Path]::GetFullPath($SourceDir)

        Get-ChildItem -LiteralPath $sourceFullPath -Recurse -File | ForEach-Object {
            $relativePath = Get-RelativePathNormalized -BasePath $sourceFullPath -TargetPath $_.FullName
            [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
                $zip,
                $_.FullName,
                $relativePath,
                [System.IO.Compression.CompressionLevel]::Optimal
            ) | Out-Null
        }
    } finally {
        $zip.Dispose()
    }
}

function Get-ResolvedCommit {
    param(
        [Parameter(Mandatory = $true)][string]$ProjectRoot,
        [string]$OverrideCommit = ""
    )

    if ($OverrideCommit) {
        return $OverrideCommit
    }

    $resolvedCommit = git -C $ProjectRoot rev-parse HEAD
    if ($LASTEXITCODE -ne 0 -or -not $resolvedCommit) {
        throw "Unable to resolve the Git commit for $ProjectRoot"
    }
    return ([string]$resolvedCommit).Trim()
}

function Write-ReleaseNotesFile {
    param(
        [Parameter(Mandatory = $true)][string]$TemplatePath,
        [Parameter(Mandatory = $true)][string]$OutputDir,
        [Parameter(Mandatory = $true)][string]$VersionText,
        [Parameter(Mandatory = $true)][string]$NativeAssetName,
        [string]$CommitText = ""
    )

    if (-not (Test-Path -LiteralPath $TemplatePath)) {
        throw "Release notes template does not exist: $TemplatePath"
    }

    $content = [System.IO.File]::ReadAllText($TemplatePath, [System.Text.Encoding]::UTF8)
    $replacements = @{
        "{{VERSION}}" = $VersionText
        "{{COMMIT}}" = $CommitText
        "{{NATIVE_ASSET_NAME}}" = $NativeAssetName
    }

    foreach ($entry in $replacements.GetEnumerator()) {
        $content = $content.Replace($entry.Key, $entry.Value)
    }

    $outputPath = Join-Path $OutputDir "release-notes-v$VersionText.md"
    [System.IO.File]::WriteAllText($outputPath, $content, [System.Text.UTF8Encoding]::new($false))
    Write-Host "Created release notes: $outputPath"
}

function Package-NativeArtifacts {
    param(
        [Parameter(Mandatory = $true)][string]$PrebuiltSourceDir,
        [Parameter(Mandatory = $true)][string]$HeadersSourceDir,
        [Parameter(Mandatory = $true)][string]$OutputDir,
        [Parameter(Mandatory = $true)][string]$VersionText,
        [Parameter(Mandatory = $true)][string[]]$Platforms,
        [Parameter(Mandatory = $true)][string[]]$RequiredFiles,
        [string]$CommitText = "",
        [switch]$Overwrite
    )

    if (-not (Test-Path -LiteralPath $PrebuiltSourceDir)) {
        throw "Prebuilt source directory does not exist: $PrebuiltSourceDir"
    }
    if (-not (Test-Path -LiteralPath $HeadersSourceDir)) {
        throw "Headers source directory does not exist: $HeadersSourceDir"
    }

    $missingPrebuiltFiles = @($RequiredFiles | Where-Object {
        $relativePath = $_ -replace '/', [System.IO.Path]::DirectorySeparatorChar
        -not (Test-Path -LiteralPath (Join-Path $PrebuiltSourceDir $relativePath) -PathType Leaf)
    })
    if ($missingPrebuiltFiles.Count -gt 0) {
        throw "Required prebuilt files are missing: $($missingPrebuiltFiles -join ', ')"
    }

    $headerFiles = @(
        Get-ChildItem -LiteralPath $HeadersSourceDir -Recurse -File |
            Where-Object { $_.Extension -in @(".h", ".hpp") }
    )
    if ($headerFiles.Count -eq 0) {
        throw "No header files were found under $HeadersSourceDir"
    }

    $archiveName = "sweetline-native-v$VersionText.zip"
    $archivePath = Join-Path $OutputDir $archiveName
    if ((Test-Path -LiteralPath $archivePath) -and -not $Overwrite) {
        throw "Archive already exists: $archivePath. Use -Force to overwrite it."
    }

    $tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("sweetline-native-" + [System.Guid]::NewGuid().ToString("N"))
    $stageDir = Join-Path $tempRoot "stage"
    $stagePrebuiltDir = Join-Path $stageDir "prebuilt"
    $stageIncludeDir = Join-Path $stageDir "include"
    Ensure-Directory -Path $stagePrebuiltDir
    Ensure-Directory -Path $stageIncludeDir

    try {
        foreach ($platformName in $Platforms) {
            $sourcePlatformDir = Join-Path $PrebuiltSourceDir $platformName
            $stagePlatformDir = Join-Path $stagePrebuiltDir $platformName
            Copy-DirectoryContents -Source $sourcePlatformDir -Destination $stagePlatformDir
        }

        foreach ($file in $headerFiles) {
            $relativePath = Get-RelativePathNormalized -BasePath $HeadersSourceDir -TargetPath $file.FullName
            $destinationPath = Join-Path $stageIncludeDir ($relativePath -replace '/', [System.IO.Path]::DirectorySeparatorChar)
            Ensure-Directory -Path (Split-Path -Parent $destinationPath)
            Copy-Item -LiteralPath $file.FullName -Destination $destinationPath -Force
        }

        Write-NativeReadmeFile -Path (Join-Path $stageDir "README.txt") -VersionText $VersionText -Platforms $Platforms -CommitText $CommitText
        Write-ChecksumsFile -StageDir $stageDir

        New-ZipFromDirectoryContents -SourceDir $stageDir -ZipPath $archivePath
        Write-Host "Created native SDK archive: $archivePath"
    } finally {
        Remove-PathIfExists -Path $tempRoot
    }
}

Ensure-Directory -Path $ResolvedOutputDir
$resolvedCommit = Get-ResolvedCommit -ProjectRoot $ProjectDir -OverrideCommit $Commit
$nativeArchiveName = "sweetline-native-v$Version.zip"

Package-NativeArtifacts `
    -PrebuiltSourceDir $ResolvedPrebuiltSourceDir `
    -HeadersSourceDir $ResolvedHeadersSourceDir `
    -OutputDir $ResolvedOutputDir `
    -VersionText $Version `
    -Platforms $RequiredPlatforms `
    -RequiredFiles $RequiredPrebuiltFiles `
    -CommitText $resolvedCommit `
    -Overwrite:$Force

if (-not $SkipReleaseNotes) {
    Write-ReleaseNotesFile `
        -TemplatePath $ResolvedReleaseNotesTemplate `
        -OutputDir $ResolvedOutputDir `
        -VersionText $Version `
        -NativeAssetName $nativeArchiveName `
        -CommitText $resolvedCommit
}
