# Tower Shooter Build and Source Identification System

**Purpose:**  
Provide a reliable, human-readable way to identify the exact source revision and packaged build that produced a gameplay telemetry report or performance benchmark.

This system deliberately does **not** use Unreal Engine's internal `BuildId`. Unreal's `BuildId` is used for binary/module compatibility and is not intended to be a human-facing project build identifier.

---

## 1. Goals

The system must make it possible to answer:

- Which Git revision produced this run?
- Were there uncommitted changes when the run/build was created?
- Was this an Editor/PIE run or a packaged build?
- If packaged, which specific package was used?
- What configuration and platform was packaged?
- Can the packaged build still identify itself if it is moved elsewhere or Git is unavailable?

The final information is consumed by the existing Blueprint-based telemetry and performance reporting systems.

---

## 2. Identifier Model

### Source Revision

The source revision identifies the Git source state.

Example:

```text
543-864721b
```

This is composed from:

- `SourceCommitCount` — a human-readable count of commits reachable from `HEAD`
- `SourceShortHash` — the seven-character Git commit hash

Example:

```text
SourceCommitCount: 543
SourceShortHash:   864721b
SourceRevision:    543-864721b
```

The full Git hash is also stored:

```text
864721b1fbc0a594819993d0c6070677e6ee6f63
```

### Important: the hash is the canonical identity

The commit count is useful for human orientation, but it is not a permanent Git identifier. Rebasing, squashing, or otherwise rewriting history can change commit counts.

The full Git hash is therefore the canonical source identifier.

### Dirty State

`SourceDirty` records whether Git reported uncommitted changes at the time the source information was captured.

For example:

```text
SourceRevision: 543-864721b
SourceDirty:    True
```

This matters because a dirty working tree cannot necessarily be reproduced by checking out the reported commit alone.

### Build ID

Packaged builds receive a separate Build ID.

Current format:

```text
YYYYMMDD-NNN
```

Example:

```text
20260821-002
```

The sequence resets each day and increments whenever a packaging attempt begins.

Gaps are allowed. A failed package may consume a Build ID.

The Build ID identifies the **packaged artifact**, while the source revision identifies the **Git source state**.

Multiple packaged builds may therefore legitimately share the same source revision:

```text
SourceRevision: 543-864721b
BuildID:        20260821-001
```

and:

```text
SourceRevision: 543-864721b
BuildID:        20260821-002
```

---

## 3. Runtime Architecture

The same Blueprint-facing function is used in both Editor and packaged builds:

```text
Get Source Revision Info
```

It returns an `FSourceRevisionInfo` struct.

Internally, the source of the information depends on execution context.

### Editor / PIE

```text
GameState BeginPlay
        |
        v
GetSourceRevisionInfo()
        |
        v
Run Git commands directly
        |
        v
FSourceRevisionInfo
        |
        v
GameState caches the result
        |
        +--> Telemetry reports
        |
        +--> Performance reports
```

### Packaged Build

```text
BuildTowerShooter.ps1
        |
        +--> Query Git
        +--> Generate Build ID
        +--> Generate BuildMetadata.json
        +--> Run Unreal packaging
        +--> Name archive directory
        +--> Copy BuildMetadata.json into package
                    |
                    v
              Packaged game
                    |
                    v
          GetSourceRevisionInfo()
                    |
                    v
          GetPackagedBuildInfo()
                    |
                    v
          Read BuildMetadata.json
                    |
                    v
            FSourceRevisionInfo
                    |
                    v
          GameState caches result
```

A packaged build never needs Git to be installed.

---

## 4. Why the GameState Caches the Information

`GetSourceRevisionInfo()` is called once at the start of `GameState::BeginPlay` / the Blueprint equivalent, and the returned struct is stored on the GameState.

Other systems read the cached struct rather than querying the source information independently.

This is intentional.

The Editor implementation launches several external Git processes. This is trivial when performed once at startup, but there is no reason for ReportManager, PerformanceTestManager, or other systems to repeat the same work.

The cached information also represents a useful conceptual snapshot:

> This is the source/build state associated with this play session.

Do not call the Git-query function repeatedly or on Tick.

---

## 5. C++ Files

The implementation lives in:

```text
Source/TowerShooter/Public/TowerShooterBuildInfoLibrary.h
Source/TowerShooter/Private/TowerShooterBuildInfoLibrary.cpp
```

The exact Public/Private layout may vary if the project structure changes, but the class is:

```cpp
UTowerShooterBuildInfoLibrary
```

and the Blueprint-facing struct is:

```cpp
FSourceRevisionInfo
```

---

## 6. FSourceRevisionInfo

The struct contains source-control and packaged-build metadata.

Current fields include:

```text
bValid
CommitCount
ShortHash
FullHash
Branch
bDirty
RevisionDisplay

BuildID
BuildTimestampUTC
BuildConfiguration
BuildPlatform
Execution

Error
```

### Field meanings

`bValid`  
True only if all information required for the active execution path was retrieved successfully.

`CommitCount`  
Commit count returned by:

```text
git rev-list --count HEAD
```

Used for human-readable source revision display.

`ShortHash`  
Seven-character abbreviated Git hash.

`FullHash`  
Full Git commit hash. This is the canonical source identity.

`Branch`  
Current Git branch at the time the information was captured. Diagnostic context only; it is not part of the canonical source identity.

`bDirty`  
True if `git status --porcelain` returns any output.

`RevisionDisplay`  
Human-readable combined revision:

```text
<CommitCount>-<ShortHash>
```

Example:

```text
543-864721b
```

`BuildID`  
Unique packaged-build identifier. Empty for Editor/PIE runs.

`BuildTimestampUTC`  
UTC time recorded by the packaging script when the build metadata was generated.

`BuildConfiguration`  
The configuration requested by the packaging script, currently typically:

```text
Development
```

`BuildPlatform`  
The packaging target, currently:

```text
Win64
```

`Execution`  
Current values:

```text
Editor
Packaged
```

`Error`  
Diagnostic information if `bValid == false`.

---

## 7. Editor Git Queries

Editor/PIE runs call Git directly through:

```cpp
RunGitCommand(...)
```

This helper uses:

```cpp
FPlatformProcess::ExecProcess
```

to run `git.exe`.

The working directory is Unreal's project directory:

```cpp
FPaths::ProjectDir()
```

Git automatically searches upward through parent directories until it finds the repository's `.git` directory.

This is useful because the Unreal project currently lives below the repository root.

The Git commands currently used are:

```text
git rev-parse HEAD
git rev-parse --short=7 HEAD
git rev-list --count HEAD
git branch --show-current
git status --porcelain
```

### Git executable requirement

Editor source lookup relies on `git.exe` being available through the Windows `PATH`.

Git for Windows is installed as a normal system installation.

SourceTree has also been switched to use **System Git**, so SourceTree, PowerShell, and Unreal use the same Git installation.

### Important environment-variable behaviour

Applications inherit their environment variables when they launch.

If Git is installed or the system `PATH` changes while Epic Games Launcher is already running, Unreal launched from that existing Launcher process may not see `git.exe`.

Observed symptom:

```text
bValid = false
Error = Failed to query Git revision
```

with no useful Git error appended.

Resolution:

1. Close Unreal Editor.
2. Completely exit Epic Games Launcher.
3. Restart Epic Games Launcher.
4. Relaunch Unreal.

A Windows restart also refreshes the environment if required.

---

## 8. Editor vs Packaged Code Selection

`GetSourceRevisionInfo()` uses conditional compilation.

Conceptually:

```cpp
#if !WITH_EDITOR

    return GetPackagedBuildInfo();

#else

    // Query Git directly.
    ...

#endif
```

This matters because simply returning early in the packaged build while leaving the Editor implementation compiled below it caused Unreal's packaged Development compilation to report `C4702: unreachable code` as an error.

The `#if / #else / #endif` structure ensures that the irrelevant implementation is not compiled into that execution path.

---

## 9. PowerShell Build Wrapper

The build script lives at:

```text
BuildScripts/BuildTowerShooter.ps1
```

A small batch file may be used as a double-click launcher:

```text
BuildScripts/BuildTowerShooter.bat
```

The PowerShell script is intentionally a lightweight wrapper around Unreal's existing packaging infrastructure.

It does **not** implement a replacement build system.

It performs project-specific preparation, then invokes:

```text
RunUAT.bat BuildCookRun
```

which is the same underlying Unreal AutomationTool pipeline used by packaging from the Editor.

A scripted build was successfully tested and did not introduce a second cook/build stage.

---

## 10. PowerShell Script Responsibilities

The script currently performs the following tasks in order:

1. Read script parameters.
2. Establish repository, Unreal-project, engine, and UAT paths.
3. Query Git.
4. Construct `SourceRevision`.
5. Read/update the local Build ID counter.
6. Construct the Build ID.
7. Construct the packaged archive directory.
8. Generate `BuildMetadata.json`.
9. Run Unreal AutomationTool / BuildCookRun.
10. Detect package failure through `$LASTEXITCODE`.
11. Locate the packaged executable/output.
12. Copy `BuildMetadata.json` into the packaged build.
13. Print the resulting build information.

---

## 11. PowerShell Parameters

The script accepts:

```powershell
param(
    [string]$Configuration = "Development",
    [string]$Platform = "Win64"
)
```

The same values drive both Unreal packaging and metadata generation.

This is intentional: metadata should describe the build that was actually requested rather than duplicating configuration as unrelated hard-coded strings.

Normal usage:

```powershell
.\BuildTowerShooter.ps1
```

uses:

```text
Development
Win64
```

A different configuration could be requested with:

```powershell
.\BuildTowerShooter.ps1 -Configuration Shipping
```

if/when required.

---

## 12. Local Build Counter

The Build ID state is stored under:

```text
TowerShooter/BuildLocal/
```

Current counter file:

```text
TowerShooter/BuildLocal/BuildCounter.json
```

Example:

```json
{
    "Date": "20260821",
    "LastBuild": 2
}
```

`BuildLocal` is intentionally listed in `.gitignore`.

### Why this is not stored in Saved

The counter should survive normal Unreal `Saved`-directory cleanup.

It is project-local state but is not source data.

Therefore:

```text
BuildLocal
```

is treated as local, persistent, Git-ignored build state.

### Counter policy

- Resets to 1 when the date changes.
- Increments whenever the script is run for a new packaging attempt.
- A failed package may leave a gap in numbering.
- Gaps are acceptable.
- The counter is not intended to be an archival ledger.

---

## 13. Build Metadata

Before packaging, the script creates:

```text
BuildLocal/BuildMetadata.json
```

This file is staging data.

Example contents:

```json
{
    "SchemaVersion": 1,
    "SourceRevision": "543-864721b",
    "SourceCommitCount": 543,
    "SourceShortHash": "864721b",
    "SourceCommit": "864721b1fbc0a594819993d0c6070677e6ee6f63",
    "SourceBranch": "main",
    "SourceDirty": true,
    "BuildID": "20260821-002",
    "BuildTimestampUTC": "2026-08-21T00:08:21Z",
    "BuildConfiguration": "Development",
    "BuildPlatform": "Win64"
}
```

`SchemaVersion` exists so the structure can be changed deliberately later if necessary.

Do not treat the staging copy in `BuildLocal` as the packaged build's permanent metadata. The build receives its own copied version.

---

## 14. Packaged Output Naming

Current archive naming convention:

```text
TowerShooter_<SourceRevision>_<BuildID>
```

Example:

```text
TowerShooter_543-864721b_20260821-002
```

This allows a package to be identified externally without launching it.

Configuration and platform are stored in metadata rather than extending the folder name unless a future workflow demonstrates a need for them there.

---

## 15. BuildMetadata.json Location in the Package

The packaging script currently places:

```text
BuildMetadata.json
```

under the package's top-level `Windows` directory.

Example conceptually:

```text
TowerShooter_543-864721b_20260821-002/
    Windows/
        BuildMetadata.json
        TowerShooter.exe
        TowerShooter/
            Binaries/
                Win64/
                    TowerShooter.exe
```

Unreal's actual running game binary is located deeper under:

```text
Windows/TowerShooter/Binaries/Win64/
```

`FPlatformProcess::ExecutablePath()` therefore returns the deeper executable path, not the top-level launcher location.

`GetPackagedBuildInfo()` resolves the metadata path by moving upward three directories:

```text
Win64
  -> Binaries
  -> TowerShooter
  -> Windows
```

and then appending:

```text
BuildMetadata.json
```

This path relationship is an important assumption of the current implementation.

If Unreal's packaged directory layout changes, or the script changes where it copies `BuildMetadata.json`, this lookup may need to be updated.

---

## 16. Packaged Metadata Reader

Packaged builds call:

```cpp
GetPackagedBuildInfo()
```

The function performs these steps:

1. Create an empty `FSourceRevisionInfo`.
2. Ask Unreal for the current executable path.
3. Resolve the package's `Windows` root.
4. Construct the expected path to `BuildMetadata.json`.
5. Load the file into an `FString`.
6. Deserialize the JSON into an `FJsonObject`.
7. Read all required source/build fields.
8. Set:

```text
Execution = Packaged
```

9. Set:

```text
bValid = true
```

only after all required work has completed.

If the file cannot be read:

```text
bValid = false
```

and `Error` contains the path Unreal attempted to read.

If the JSON cannot be parsed:

```text
bValid = false
Error = Failed to parse BuildMetadata.json.
```

---

## 17. JSON Module Dependency

Using Unreal's JSON classes requires both:

### C++ includes

For example:

```cpp
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
```

### Unreal module dependency

`TowerShooter.Build.cs` must include:

```csharp
"Json"
```

in the module's dependency list.

Without the `Json` module dependency the source file may compile, but linking fails with unresolved symbols such as:

```text
FJsonValue
FJsonSerializerPolicy_JsonObject
```

This is a linker problem, not a JSON syntax problem.

---

## 18. Build Failure Handling

The PowerShell script checks:

```powershell
$LASTEXITCODE
```

after running UAT.

If the exit code is non-zero:

- packaging is considered failed,
- the script reports the UAT exit code,
- the script exits rather than claiming success.

Example:

```text
Unreal packaging failed with exit code 6
```

A consumed Build ID is not rolled back.

---

## 19. Batch Launcher

A convenience batch file can invoke the PowerShell script:

```bat
@echo off

powershell.exe -NoProfile -File "%~dp0BuildTowerShooter.ps1"

echo.
pause
```

`%~dp0` resolves to the directory containing the batch file, so the launcher does not depend on the user's current working directory.

`pause` keeps the console visible after completion so the final build result or errors can be read.

---

## 20. PowerShell Execution Policy

PowerShell initially refused to run local `.ps1` scripts because script execution was disabled.

The user-level policy was changed to:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

This permits locally created scripts to run while maintaining restrictions on scripts downloaded from external sources.

The batch launcher therefore does not need to use `-ExecutionPolicy Bypass`.

---

## 21. Unreal Packaging Command

The wrapper uses Unreal AutomationTool:

```text
RunUAT.bat BuildCookRun
```

The command was derived from the successful package command generated by Unreal Editor rather than invented independently.

Important packaging arguments include the normal build/cook/stage/archive/package operations and the requested:

```text
-platform
-clientconfig
-archivedirectory
```

The wrapper deliberately omits Editor-only plumbing such as:

```text
Turnkey
-EditorIO
-EditorIOPort
```

where it is not required for the standalone invocation.

The wrapper should not introduce clean-build or clean-cook flags unless intentionally required.

This is important because frequent incremental Development builds are part of the intended workflow.

---

## 22. Telemetry Integration

The wider telemetry/reporting architecture remains Blueprint-based.

At GameState BeginPlay:

```text
Get Source Revision Info
        |
        v
Store FSourceRevisionInfo
```

Report systems then read this stored struct.

No report system should independently call Git.

Useful report fields include:

```text
SourceRevision
SourceDirty
BuildID
BuildConfiguration
BuildPlatform
Execution
```

Additional fields such as full commit hash, branch, and build timestamp remain available when more forensic detail is useful.

---

## 23. Expected Editor Result

Example:

```text
bValid:          True
RevisionDisplay: 543-864721b
Branch:          main
bDirty:          True
BuildID:
Execution:       Editor
Error:
```

An Editor run deliberately has no Build ID because it is not a packaged artifact.

---

## 24. Expected Packaged Result

Example:

```text
bValid:             True
RevisionDisplay:    543-864721b
bDirty:             True
BuildID:            20260821-002
BuildConfiguration: Development
BuildPlatform:      Win64
Execution:          Packaged
Error:
```

---

## 25. Troubleshooting

### Editor returns `bValid = false` and Git error is blank

Likely cause:

`git.exe` is not visible through the environment inherited by Unreal.

Check:

```powershell
git --version
```

If PowerShell sees Git but Unreal does not, completely restart Epic Games Launcher and Unreal Editor.

---

### Editor reports `fatal: not a git repository`

Check that:

```cpp
FPaths::ProjectDir()
```

is inside the repository hierarchy.

Git should search parent directories automatically.

---

### Packaged build returns `bValid = false`

Print/read:

```text
FSourceRevisionInfo.Error
```

Do not diagnose only from empty metadata fields.

If the error begins:

```text
Failed to read packaged build metadata:
```

compare the reported path with the actual location of `BuildMetadata.json`.

The current implementation expects the metadata in the package's `Windows` root and assumes the running binary is three directories deeper.

---

### Packaged build cannot parse JSON

Check the packaged copy of:

```text
BuildMetadata.json
```

manually.

Ensure it is valid JSON and that the PowerShell script completed the metadata-generation step before packaging/copying.

---

### C++ compiles but fails to link with JSON symbols

Ensure:

```csharp
"Json"
```

exists in `TowerShooter.Build.cs`.

---

### Packaged compilation reports unreachable code (`C4702`)

Ensure `GetSourceRevisionInfo()` uses:

```cpp
#if !WITH_EDITOR
    ...
#else
    ...
#endif
```

rather than an unconditional packaged return followed by Editor code.

---

### Build succeeds but metadata is missing from the package

Check the PowerShell post-package copy step.

Also confirm the script found the correct packaged executable/output directory before copying the metadata file.

---

### Build folder and metadata Build ID disagree

This indicates ordering or variable reuse in the PowerShell script.

`SourceRevision` and `BuildID` must both be generated **before** `$ArchiveDirectory` and `$BuildMetadata` are assembled.

PowerShell string interpolation happens when the assignment executes; earlier strings do not automatically update when referenced variables change later.

---

## 26. Known Assumptions

The current implementation assumes:

- Windows development environment.
- Git for Windows is installed and available on `PATH`.
- Unreal Engine 5.7 installation path is configured in the PowerShell script.
- Current main packaging target is Win64.
- The Unreal project directory is below the Git repository root.
- The PowerShell script knows the project-relative Unreal directory.
- `BuildMetadata.json` is copied to the packaged `Windows` root.
- The real running executable resides at `Windows/TowerShooter/Binaries/Win64`.
- Packaged builds are created through the project's PowerShell wrapper when Build IDs are required.

These assumptions are appropriate for the current solo-development workflow. They should only be generalized when an actual need appears.

---

## 27. Scope Boundaries

This system is intentionally **not**:

- a CI pipeline,
- a release-management service,
- a replacement for Unreal AutomationTool,
- a semantic versioning system,
- a save-game compatibility version,
- a network protocol version,
- Unreal's internal binary `BuildId`,
- or a general build database.

It exists to answer one practical development question reliably:

> Exactly which source state and packaged artifact produced this run or benchmark?

Do not expand the system unless a concrete workflow requirement justifies the additional complexity.

---

## 28. Files to Check When Something Breaks

Start with these files:

```text
BuildScripts/BuildTowerShooter.ps1
BuildScripts/BuildTowerShooter.bat

TowerShooter/BuildLocal/BuildCounter.json
TowerShooter/BuildLocal/BuildMetadata.json

Source/TowerShooter/Public/TowerShooterBuildInfoLibrary.h
Source/TowerShooter/Private/TowerShooterBuildInfoLibrary.cpp
Source/TowerShooter/TowerShooter.Build.cs
```

Also inspect the packaged copy:

```text
<BuildArchive>/Windows/BuildMetadata.json
```

and the Blueprint GameState location where `FSourceRevisionInfo` is cached.

---

## 29. Maintenance Rule

When changing this system, preserve the separation of responsibilities:

### Git
Identifies source.

### PowerShell wrapper
Creates packaged-build identity and orchestrates Unreal's normal packaging pipeline.

### BuildMetadata.json
Transfers immutable build-time information into the packaged artifact.

### C++ helper
Bridges external build/source metadata into Unreal.

### GameState
Caches the identity snapshot once per play session.

### Blueprint telemetry
Consumes and reports the information.

Keeping these responsibilities separate should make future problems much easier to isolate.
