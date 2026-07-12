#pragma once

#include "RecoveryEngine/Core/Types.h"

#include <mutex>
#include <string>

struct sqlite3;

namespace recovery {

class SessionStore {
public:
    explicit SessionStore(std::string dbPath);
    ~SessionStore();

    bool Initialize();
    bool Save(const SessionSnapshot& snapshot);
    bool Load(const std::string& sessionId, SessionSnapshot& snapshot);

private:
    bool Exec(const char* sql) const;

    std::string dbPath_;
    sqlite3* db_{};
    mutable std::mutex mutex_;
};

}  // namespace recovery
