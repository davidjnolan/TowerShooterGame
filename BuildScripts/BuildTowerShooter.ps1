param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64"
)

$ProjectRoot = Split-Path -Parent $PSScriptRoot

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
$UnrealProjectRoot = Join-Path $ProjectRoot "TowerShooter"

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



# Print
Write-Host ""
Write-Host "Source Revision: $SourceRevision"
Write-Host "Full Hash:       $FullHash"
Write-Host "Branch:          $Branch"
Write-Host "Dirty:           $IsDirty"
Write-Host "Build ID:        $BuildID"

