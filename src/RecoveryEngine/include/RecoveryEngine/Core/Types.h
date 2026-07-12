#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace recovery {

enum class FileSystemType : uint8_t {
    Unknown,
    NTFS,
    FAT32,
    ExFAT,
    ReFS
};

enum class ScanMode : uint8_t {
    Quick,
    Deep,
    Custom
};

enum class FileHealth : uint8_t {
    Good,
    Poor,
    Overwritten,
    Unrecoverable
};

struct DiskGeometry {
    uint64_t totalBytes{};
    uint32_t bytesPerSector{512};
    uint32_t sectorsPerCluster{8};
};

struct ReadRequest {
    uint64_t offset{};
    uint32_t size{};
};

struct ScanFilter {
    std::vector<std::string> includeExtensions;
    uint64_t minSize{};
    uint64_t maxSize{UINT64_MAX};
    std::optional<std::chrono::system_clock::time_point> modifiedAfter;
    std::optional<std::chrono::system_clock::time_point> modifiedBefore;
};

struct RecoverableFile {
    uint64_t id{};
    std::string path;
    std::string extension;
    uint64_t size{};
    FileHealth health{FileHealth::Good};
    bool deleted{};
    bool carved{};
    uint64_t sourceOffset{};
};

struct ScanProgress {
    uint64_t bytesScanned{};
    uint64_t totalBytes{};
    uint32_t filesFound{};
    bool paused{};
    bool cancelled{};
};

struct SectorError {
    uint64_t offset{};
    uint32_t length{};
    uint32_t osErrorCode{};
    std::string message;
};

struct SessionSnapshot {
    std::string sessionId;
    std::string source;
    std::string destination;
    ScanMode mode{ScanMode::Quick};
    ScanProgress progress;
    std::vector<RecoverableFile> files;
    std::vector<SectorError> errors;
};

}  // namespace recovery
