# Data-Recovery

Professional-grade Windows-focused data recovery solution with a native C++17 recovery engine and a .NET 8 WPF frontend.

## Repository Layout

- `/src/RecoveryEngine` - C++17 shared library implementing low-level I/O, parser plugins, carving, session persistence, imaging, RAID mapping, and C API interop.
- `/src/UI/RecoveryApp` - WPF desktop application using P/Invoke to call RecoveryEngine.dll.
- `/tests/RecoveryEngineTests` - GoogleTest-based unit tests for core native modules.
- `/tests/RecoveryApp.Tests` - xUnit tests for managed client behavior.
- `/data/signatures/default_signatures.json` - configurable signature database with 20+ file signatures.
- `/docs/ARCHITECTURE.md` - architecture and subsystem contracts.

## Feature Coverage

### Implemented

- Strict read-only source handling interfaces (`IDiskAccess`) with physical drive and raw image access implementations.
- Sector-aligned synchronous reads and asynchronous dispatch wrappers.
- File system parser plugin model (`IFileSystemParser`) with NTFS/FAT32/exFAT/ReFS probes.
- Signature-driven carving engine with Aho-Corasick multi-pattern matching.
- Recovery orchestration and pause/resume/cancel wiring (`RecoveryManager`).
- SQLite-backed session persistence with transactional save/load.
- Forensic raw imaging pipeline.
- RAID 0/1/5 logical read mapping module.
- VSS and BitLocker integration adapter interfaces for Windows execution paths.
- Native C API and C# P/Invoke bridge.
- WPF operator UI with source/destination/session controls, real-time results grid scaffolding, preview column, and built-in hex viewer panel.

### Safety Model

- Recovery sources are opened read-only.
- Physical disk access uses `GENERIC_READ` and share-read/write/delete.
- Recovery output paths are separate and explicit.
- Session state writes are transactional using SQLite WAL mode.

## Build Prerequisites

### Windows (Recommended)

- Windows 10/11 x64 (Windows 7 x64 possible with compatible toolset and SDK)
- Visual Studio 2022 (Desktop development with C++)
- CMake 3.24+
- Windows 10/11 SDK
- .NET SDK 8.0
- Git (for FetchContent dependencies)

### Native dependencies fetched by CMake

- `spdlog`
- `nlohmann/json`
- `googletest` (for tests)

### System dependency

- SQLite3 development package/libraries available to CMake (`find_package(SQLite3 REQUIRED)`)

## Build Guide

### 1) Build native engine + native tests

```powershell
cd <repo-root>
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure -C Release
```

### 2) Build WPF UI

```powershell
cd <repo-root>\src\UI\RecoveryApp
dotnet restore
dotnet build -c Release
```

### 3) Run managed tests

```powershell
cd <repo-root>
dotnet test tests\RecoveryApp.Tests\RecoveryApp.Tests.csproj -c Release
```

## Usage Workflow

1. Run the WPF app elevated (Administrator).
2. Choose source (`\\.\PhysicalDriveX` or image path).
3. Choose destination folder on healthy media.
4. Start scan (quick/deep/custom mode wiring available through interop).
5. Pause/resume scan and save/load session state.
6. Review file list and preview fields.
7. Recover selected files to destination path.

## Logging and Diagnostics

- Recovery session metadata is persisted in SQLite (`sessions.db` by default).
- Scan snapshots include progress, file candidates, and sector error inventory.
- Extend logging by adding `spdlog` sinks in the RecoveryEngine layer.

## Limitations and Operational Notes

- Parser probe logic is implemented; full deep metadata reconstruction for every filesystem variant is scaffolded for extension in parser modules.
- VSS and BitLocker adapters are wired for Windows integration points and can be expanded with environment-specific credential and policy flows.
- Physical-drive I/O requires administrator privileges and compatible storage drivers.
- Always validate recovered file integrity before production use.

## Security and Forensics Notes

- Use write-blocked acquisition workflows where required by policy.
- Perform sector imaging before aggressive deep scan operations on unstable media.
- Preserve generated session DB and logs for audit trail retention.
