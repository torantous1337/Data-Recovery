#pragma once

#include "RecoveryEngine/Carving/CarvingEngine.h"
#include "RecoveryEngine/Core/Types.h"
#include "RecoveryEngine/Parsers/IFileSystemParser.h"
#include "RecoveryEngine/Persistence/SessionStore.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace recovery {

class RecoveryManager {
public:
    using ProgressCallback = std::function<void(const ScanProgress&)>;

    RecoveryManager(std::unique_ptr<IFileSystemParser> parser,
                    std::unique_ptr<CarvingEngine> carver,
                    std::shared_ptr<SessionStore> sessionStore);
    ~RecoveryManager();

    bool StartScan(IDiskAccess& disk,
                   const std::string& sessionId,
                   ScanMode mode,
                   const ScanFilter& filter,
                   ProgressCallback onProgress);
    void Pause();
    void Resume();
    void Cancel();
    SessionSnapshot Snapshot() const;

private:
    void ScanThread(IDiskAccess* disk, ScanFilter filter, ProgressCallback onProgress);

    std::unique_ptr<IFileSystemParser> parser_;
    std::unique_ptr<CarvingEngine> carver_;
    std::shared_ptr<SessionStore> sessionStore_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::thread worker_;
    std::atomic<bool> paused_{false};
    std::atomic<bool> cancelled_{false};
    SessionSnapshot snapshot_;
};

}  // namespace recovery
