# Architecture Overview

## Layered Components

1. **Low-Level I/O Layer**
   - `IDiskAccess` defines source abstraction.
   - `PhysicalDriveAccess` handles `\\.\PhysicalDriveX` with sector-aligned reads.
   - `RawImageAccess` handles forensic raw image files.

2. **File System Parser Plugins**
   - `IFileSystemParser` contract.
   - Built-ins: NTFS, FAT32, exFAT, ReFS probes.
   - `ParserFactory` auto-detects or applies manual override.

3. **Carving Engine**
   - `SignatureDatabase` loads JSON signatures.
   - `AhoCorasick` performs fast multi-signature search.
   - `CarvingEngine` extracts and validates carved candidates.

4. **Recovery Manager**
   - Coordinates parser and carver.
   - Tracks scan progress and state.
   - Supports pause/resume/cancel controls.

5. **Persistence Layer**
   - `SessionStore` stores snapshots in SQLite with transaction boundaries.

6. **Advanced Recovery Modules**
   - `ImagingEngine` raw dd-style imaging.
   - `RaidReconstructor` RAID 0/1/5 logical mapping.
   - `VssProvider`, `BitLockerAdapter` integration points.

7. **Interop and UI**
   - C API: `RecoveryEngineC.h` exports ABI-stable entry points.
   - C# client uses P/Invoke wrappers.
   - WPF front-end follows VM-driven presentation (`MainViewModel`).

## Data Flow

1. UI gathers source, destination, and scan options.
2. UI calls native C API start scan.
3. Engine opens source read-only and selects parser.
4. Parser metadata pass + carving pass produce candidates.
5. Recovery state is serialized to SQLite.
6. UI can pause/resume/save/load sessions.

## Threading Model

- Engine exposes asynchronous read API and worker-thread scan execution.
- Pause/resume signaling uses atomic flags and condition variables.
- Module boundaries isolate I/O, parse, carve, and persistence responsibilities.

## Extensibility

- Add new parser by implementing `IFileSystemParser` and registering in `ParserFactory`.
- Add new file signatures in `data/signatures/default_signatures.json`.
- Extend validators in `CarvingEngine::ValidateCandidate` for structure-aware checks.
- Add telemetry sinks through `spdlog` integration.
