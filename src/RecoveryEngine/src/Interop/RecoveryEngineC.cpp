#include "RecoveryEngine/Interop/RecoveryEngineC.h"

#include "RecoveryEngine/Carving/CarvingEngine.h"
#include "RecoveryEngine/Carving/SignatureDatabase.h"
#include "RecoveryEngine/IO/IDiskAccess.h"
#include "RecoveryEngine/Parsers/ParserFactory.h"
#include "RecoveryEngine/Persistence/SessionStore.h"
#include "RecoveryEngine/Recovery/RecoveryManager.h"

#include <memory>
#include <string>
#include <algorithm>
#include <cstring>

using namespace recovery;

struct RecoveryHandle {
    std::string lastError;
    std::unique_ptr<SignatureDatabase> signatureDb;
    std::shared_ptr<SessionStore> sessionStore;
    std::unique_ptr<RecoveryManager> manager;
    std::unique_ptr<IDiskAccess> disk;
};

RecoveryHandle* Recovery_Create() {
    auto* handle = new RecoveryHandle();
    handle->signatureDb = std::make_unique<SignatureDatabase>();
    return handle;
}

void Recovery_Destroy(RecoveryHandle* handle) {
    delete handle;
}

int Recovery_StartScan(RecoveryHandle* handle, const RecoveryScanOptions* options) {
    if (!handle || !options) {
        return -1;
    }
    if (!handle->signatureDb->LoadFromJson(options->signatureDb ? options->signatureDb : "data/signatures/default_signatures.json")) {
        handle->lastError = "Unable to load signature database";
        return -2;
    }

    handle->sessionStore = std::make_shared<SessionStore>(options->sessionDb ? options->sessionDb : "sessions.db");
    if (!handle->sessionStore->Initialize()) {
        handle->lastError = "Unable to initialize session store";
        return -3;
    }

    const std::string source = options->source ? options->source : "";
    if (source.rfind("\\\\.\\PhysicalDrive", 0) == 0) {
        handle->disk = std::make_unique<PhysicalDriveAccess>();
    } else {
        handle->disk = std::make_unique<RawImageAccess>();
    }

    if (!handle->disk->OpenReadOnly(source)) {
        handle->lastError = "Unable to open source in read-only mode";
        return -4;
    }

    auto parser = ParserFactory::CreateForDisk(*handle->disk);
    if (!parser) {
        handle->lastError = "Unsupported file system";
        return -5;
    }

    auto carver = std::make_unique<CarvingEngine>(handle->signatureDb.get());
    handle->manager = std::make_unique<RecoveryManager>(std::move(parser), std::move(carver), handle->sessionStore);

    ScanFilter filter;
    const auto sessionId = std::string("session-") + std::to_string(reinterpret_cast<uintptr_t>(handle));
    if (!handle->manager->StartScan(*handle->disk, sessionId, static_cast<ScanMode>(options->mode), filter, nullptr)) {
        handle->lastError = "Scan is already running";
        return -6;
    }

    return 0;
}

int Recovery_Pause(RecoveryHandle* handle) {
    if (!handle || !handle->manager) return -1;
    handle->manager->Pause();
    return 0;
}

int Recovery_Resume(RecoveryHandle* handle) {
    if (!handle || !handle->manager) return -1;
    handle->manager->Resume();
    return 0;
}

int Recovery_Cancel(RecoveryHandle* handle) {
    if (!handle || !handle->manager) return -1;
    handle->manager->Cancel();
    return 0;
}

int Recovery_SaveSession(RecoveryHandle* handle, const char* sessionId) {
    if (!handle || !handle->manager || !handle->sessionStore) return -1;
    auto snapshot = handle->manager->Snapshot();
    if (sessionId) {
        snapshot.sessionId = sessionId;
    }
    return handle->sessionStore->Save(snapshot) ? 0 : -2;
}

int Recovery_LoadSession(RecoveryHandle* handle, const char* sessionId) {
    if (!handle || !handle->sessionStore || !sessionId) return -1;
    SessionSnapshot snapshot;
    return handle->sessionStore->Load(sessionId, snapshot) ? 0 : -2;
}

int Recovery_GetLastError(RecoveryHandle* handle, char* buffer, uint32_t bufferLen) {
    if (!handle || !buffer || bufferLen == 0) return -1;
    const auto toCopy = std::min<size_t>(handle->lastError.size(), bufferLen - 1);
    std::memcpy(buffer, handle->lastError.data(), toCopy);
    buffer[toCopy] = '\0';
    return 0;
}
