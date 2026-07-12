#include "RecoveryEngine/Persistence/SessionStore.h"

#include <sqlite3.h>

namespace recovery {

SessionStore::SessionStore(std::string dbPath) : dbPath_(std::move(dbPath)) {}

SessionStore::~SessionStore() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool SessionStore::Exec(const char* sql) const {
    char* err{};
    const auto rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
    return rc == SQLITE_OK;
}

bool SessionStore::Initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        return false;
    }
    return Exec("PRAGMA journal_mode=WAL;") &&
           Exec("CREATE TABLE IF NOT EXISTS sessions(session_id TEXT PRIMARY KEY, source TEXT, destination TEXT, mode INTEGER, bytes_scanned INTEGER, total_bytes INTEGER, files_found INTEGER, paused INTEGER, cancelled INTEGER);") &&
           Exec("CREATE TABLE IF NOT EXISTS files(session_id TEXT, file_id INTEGER, path TEXT, extension TEXT, size INTEGER, health INTEGER, deleted INTEGER, carved INTEGER, source_offset INTEGER, PRIMARY KEY(session_id, file_id));") &&
           Exec("CREATE TABLE IF NOT EXISTS errors(session_id TEXT, offset INTEGER, length INTEGER, os_error INTEGER, message TEXT);");
}

bool SessionStore::Save(const SessionSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_) {
        return false;
    }

    Exec("BEGIN TRANSACTION;");
    sqlite3_stmt* stmt{};
    sqlite3_prepare_v2(db_, "REPLACE INTO sessions(session_id, source, destination, mode, bytes_scanned, total_bytes, files_found, paused, cancelled) VALUES (?,?,?,?,?,?,?,?,?);", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, snapshot.sessionId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, snapshot.source.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, snapshot.destination.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, static_cast<int>(snapshot.mode));
    sqlite3_bind_int64(stmt, 5, static_cast<sqlite3_int64>(snapshot.progress.bytesScanned));
    sqlite3_bind_int64(stmt, 6, static_cast<sqlite3_int64>(snapshot.progress.totalBytes));
    sqlite3_bind_int(stmt, 7, static_cast<int>(snapshot.progress.filesFound));
    sqlite3_bind_int(stmt, 8, snapshot.progress.paused ? 1 : 0);
    sqlite3_bind_int(stmt, 9, snapshot.progress.cancelled ? 1 : 0);
    const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db_, "DELETE FROM files WHERE session_id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, snapshot.sessionId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db_, "DELETE FROM errors WHERE session_id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, snapshot.sessionId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db_, "INSERT INTO files(session_id, file_id, path, extension, size, health, deleted, carved, source_offset) VALUES (?,?,?,?,?,?,?,?,?);", -1, &stmt, nullptr);
    for (const auto& f : snapshot.files) {
        sqlite3_bind_text(stmt, 1, snapshot.sessionId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(f.id));
        sqlite3_bind_text(stmt, 3, f.path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, f.extension.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 5, static_cast<sqlite3_int64>(f.size));
        sqlite3_bind_int(stmt, 6, static_cast<int>(f.health));
        sqlite3_bind_int(stmt, 7, f.deleted ? 1 : 0);
        sqlite3_bind_int(stmt, 8, f.carved ? 1 : 0);
        sqlite3_bind_int64(stmt, 9, static_cast<sqlite3_int64>(f.sourceOffset));
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db_, "INSERT INTO errors(session_id, offset, length, os_error, message) VALUES (?,?,?,?,?);", -1, &stmt, nullptr);
    for (const auto& e : snapshot.errors) {
        sqlite3_bind_text(stmt, 1, snapshot.sessionId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(e.offset));
        sqlite3_bind_int(stmt, 3, static_cast<int>(e.length));
        sqlite3_bind_int(stmt, 4, static_cast<int>(e.osErrorCode));
        sqlite3_bind_text(stmt, 5, e.message.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);

    Exec(ok ? "COMMIT;" : "ROLLBACK;");
    return ok;
}

bool SessionStore::Load(const std::string& sessionId, SessionSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_) {
        return false;
    }

    sqlite3_stmt* stmt{};
    sqlite3_prepare_v2(db_, "SELECT source, destination, mode, bytes_scanned, total_bytes, files_found, paused, cancelled FROM sessions WHERE session_id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    snapshot = {};
    snapshot.sessionId = sessionId;
    snapshot.source = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    snapshot.destination = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    snapshot.mode = static_cast<ScanMode>(sqlite3_column_int(stmt, 2));
    snapshot.progress.bytesScanned = static_cast<uint64_t>(sqlite3_column_int64(stmt, 3));
    snapshot.progress.totalBytes = static_cast<uint64_t>(sqlite3_column_int64(stmt, 4));
    snapshot.progress.filesFound = static_cast<uint32_t>(sqlite3_column_int(stmt, 5));
    snapshot.progress.paused = sqlite3_column_int(stmt, 6) == 1;
    snapshot.progress.cancelled = sqlite3_column_int(stmt, 7) == 1;
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db_, "SELECT file_id, path, extension, size, health, deleted, carved, source_offset FROM files WHERE session_id = ? ORDER BY file_id;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        RecoverableFile f;
        f.id = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
        f.path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        f.extension = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        f.size = static_cast<uint64_t>(sqlite3_column_int64(stmt, 3));
        f.health = static_cast<FileHealth>(sqlite3_column_int(stmt, 4));
        f.deleted = sqlite3_column_int(stmt, 5) == 1;
        f.carved = sqlite3_column_int(stmt, 6) == 1;
        f.sourceOffset = static_cast<uint64_t>(sqlite3_column_int64(stmt, 7));
        snapshot.files.push_back(std::move(f));
    }
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db_, "SELECT offset, length, os_error, message FROM errors WHERE session_id = ?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SectorError e;
        e.offset = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
        e.length = static_cast<uint32_t>(sqlite3_column_int(stmt, 1));
        e.osErrorCode = static_cast<uint32_t>(sqlite3_column_int(stmt, 2));
        e.message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        snapshot.errors.push_back(std::move(e));
    }
    sqlite3_finalize(stmt);

    return true;
}

}  // namespace recovery
