param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64"
)

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$UnrealProjectRoot = Join-Path $ProjectRoot "TowerShooter"
$UnrealEngineRoot = "C:\Program Files\Epic Games\UE_5.7"

$RunUAT = Join-Path $UnrealEngineRoot "Engine\Build\BatchFiles\RunUAT.bat"
$UProject = Join-Path $UnrealProjectRoot "TowerShooter.uproject"

Set-Location $ProjectRoot

$CommitCount = git rev-list --count HEAD
$ShortHash = git rev-parse --short=7 HEAD
$FullHash = git rev-parse HEAD
$Branch = git branch --show-current
$DirtyOutput = git status --porcelain

$IsDirty = -not [string]::IsNullOrWhiteSpace($DirtyOutput)

$SourceRevision = "$CommitCount-$ShortHash"


# -------------------------------------------------------------------------
# Build ID
# Format: YYYYMMDD-NNN
# Counter resets each day and increments for every packaging attempt.
# -------------------------------------------------------------------------

# Set Variables for file path
$BuildStateDirectory = Join-Path $UnrealProjectRoot "BuildLocal"
$BuildCounterFile = Join-Path $BuildStateDirectory "BuildCounter.json"

# Make directory if it doesn't exist
if (-not (Test-Path $BuildStateDirectory))
{
    New-Item -ItemType Directory -Path $BuildStateDirectory | Out-Null
}

# Get Date
$Today = Get-Date -Format "yyyyMMdd"
$BuildNumber = 1

# If BuildCounter.json exists, check the date and BuildNumber in the file.
# Increment $BuildNumber if the date matches $Today
if (Test-Path $BuildCounterFile)
{
    $CounterData = Get-Content $BuildCounterFile -Raw | ConvertFrom-Json

    if ($CounterData.Date -eq $Today)
    {
        $BuildNumber = [int]$CounterData.LastBuild + 1
    }
}


# Create $CounterData
$CounterData = @{
    Date = $Today
    LastBuild = $BuildNumber
}

#Write $CounterData to BuildCounter.json
$CounterData |
    ConvertTo-Json |
    Set-Content $BuildCounterFile

# $Assemble $BuildID
$BuildID = "{0}-{1:D3}" -f $Today, $BuildNumber

# Assemble archive output path now that SourceRevision and BuildID exist
$ArchiveDirectory = "D:\GameBuilds\TowerShooter_${SourceRevision}_${BuildID}"

# Assemble $BuildMetadata
$BuildMetadata = @{
    SchemaVersion      = 1
    SourceRevision     = $SourceRevision
    SourceCommitCount  = [int]$CommitCount
    SourceShortHash    = $ShortHash
    SourceCommit       = $FullHash
    SourceBranch       = $Branch
    SourceDirty        = $IsDirty
    BuildID            = $BuildID
    BuildTimestampUTC  = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
    BuildConfiguration = $Configuration
    BuildPlatform      = $Platform
}

# Create BuildMetadata.json
$BuildMetadataFile = Join-Path $BuildStateDirectory "BuildMetadata.json"
$BuildMetadata |
    ConvertTo-Json |
    Set-Content $BuildMetadataFile


Write-Host ""
Write-Host "Starting Unreal packaging..."
Write-Host ""

& $RunUAT `
    BuildCookRun `
    -nop4 `
    -utf8output `
    -nocompileeditor `
    -skipbuildeditor `
    -cook `
    "-project=$UProject" `
    -target=TowerShooter `
    "-unrealexe=$UnrealEngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
    "-platform=$Platform" `
    -installed `
    -SkipCookingErrorSummary `
    -stage `
    -archive `
    -package `
    -build `
    -pak `
    -iostore `
    -compressed `
    -prereqs `
    "-archivedirectory=$ArchiveDirectory" `
    "-clientconfig=$Configuration" `
    -nocompile `
    -nocompileuat

if ($LASTEXITCODE -ne 0)
{
    Write-Host ""
    Write-Error "Unreal packaging failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

# -------------------------------------------------------------------------
# Copy build metadata beside the packaged executable.
# This makes the build self-identifying both externally and at runtime.
# -------------------------------------------------------------------------

$PackagedExe = Get-ChildItem `
    -Path $ArchiveDirectory `
    -Filter "TowerShooter.exe" `
    -Recurse `
    -File |
    Select-Object -First 1

if ($null -eq $PackagedExe)
{
    Write-Error "Packaging succeeded, but TowerShooter.exe could not be found in the archive."
    exit 1
}

$PackagedRoot = $PackagedExe.Directory.FullName
$PackagedMetadataFile = Join-Path $PackagedRoot "BuildMetadata.json"

Copy-Item `
    -Path $BuildMetadataFile `
    -Destination $PackagedMetadataFile `
    -Force



Write-Host ""
Write-Host "Packaging completed successfully."
Write-Host "Build:           $ArchiveDirectory"
Write-Host "Executable:      $($PackagedExe.FullName)"
Write-Host "Build Metadata:  $PackagedMetadataFile"
Write-Host ""


# Print
Write-Host ""
Write-Host "Source Revision: $SourceRevision"
Write-Host "Full Hash:       $FullHash"
Write-Host "Branch:          $Branch"
Write-Host "Dirty:           $IsDirty"
Write-Host "Build ID:        $BuildID"

Write-Host "UAT:             $RunUAT"
Write-Host "Project:         $UProject"
Write-Host "Archive:         $ArchiveDirectory"