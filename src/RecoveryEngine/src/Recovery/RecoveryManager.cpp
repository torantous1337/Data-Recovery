#include "RecoveryEngine/Recovery/RecoveryManager.h"

namespace recovery {

RecoveryManager::RecoveryManager(std::unique_ptr<IFileSystemParser> parser,
                                 std::unique_ptr<CarvingEngine> carver,
                                 std::shared_ptr<SessionStore> sessionStore)
    : parser_(std::move(parser)), carver_(std::move(carver)), sessionStore_(std::move(sessionStore)) {}

RecoveryManager::~RecoveryManager() {
    Cancel();
    if (worker_.joinable()) {
        worker_.join();
    }
}

bool RecoveryManager::StartScan(IDiskAccess& disk,
                                const std::string& sessionId,
                                ScanMode mode,
                                const ScanFilter& filter,
                                ProgressCallback onProgress) {
    if (worker_.joinable()) {
        return false;
    }

    snapshot_ = {};
    snapshot_.sessionId = sessionId;
    snapshot_.mode = mode;
    snapshot_.source = disk.Source();
    snapshot_.progress.totalBytes = disk.Geometry().totalBytes;

    paused_ = false;
    cancelled_ = false;
    worker_ = std::thread(&RecoveryManager::ScanThread, this, &disk, filter, std::move(onProgress));
    return true;
}

void RecoveryManager::Pause() {
    paused_ = true;
    std::lock_guard<std::mutex> lock(mutex_);
    snapshot_.progress.paused = true;
}

void RecoveryManager::Resume() {
    paused_ = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot_.progress.paused = false;
    }
    cv_.notify_all();
}

void RecoveryManager::Cancel() {
    cancelled_ = true;
    cv_.notify_all();
    std::lock_guard<std::mutex> lock(mutex_);
    snapshot_.progress.cancelled = true;
}

SessionSnapshot RecoveryManager::Snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return snapshot_;
}

void RecoveryManager::ScanThread(IDiskAccess* disk, ScanFilter filter, ProgressCallback onProgress) {
    ParserContext context{disk, std::move(filter)};
    auto metadataFiles = parser_ ? parser_->EnumerateFiles(context) : std::vector<RecoverableFile>{};
    auto deletedFiles = parser_ ? parser_->RecoverDeleted(context) : std::vector<RecoverableFile>{};
    auto carvedFiles = carver_ ? carver_->Carve(*disk, 0, disk->Geometry().totalBytes) : std::vector<RecoverableFile>{};

    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot_.files = std::move(metadataFiles);
        snapshot_.files.insert(snapshot_.files.end(), deletedFiles.begin(), deletedFiles.end());
        snapshot_.files.insert(snapshot_.files.end(), carvedFiles.begin(), carvedFiles.end());
        snapshot_.progress.filesFound = static_cast<uint32_t>(snapshot_.files.size());
        snapshot_.progress.bytesScanned = snapshot_.progress.totalBytes;
    }

    if (sessionStore_) {
        sessionStore_->Save(snapshot_);
    }

    if (onProgress) {
        onProgress(snapshot_.progress);
    }
}

}  // namespace recovery
