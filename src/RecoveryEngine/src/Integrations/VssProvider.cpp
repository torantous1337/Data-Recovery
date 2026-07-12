#include "RecoveryEngine/Integrations/VssProvider.h"

#ifdef _WIN32
#include <Windows.h>
#include <Vss.h>
#include <VsBackup.h>
#include <comdef.h>
#endif

namespace recovery {

bool VssProvider::CreateSnapshot(const std::wstring& volumeName, std::wstring& snapshotDeviceObject, std::wstring& error) const {
#ifdef _WIN32
    IVssBackupComponents* backup = nullptr;
    if (FAILED(CreateVssBackupComponents(&backup)) || backup == nullptr) {
        error = L"CreateVssBackupComponents failed";
        return false;
    }

    backup->InitializeForBackup();
    backup->SetContext(VSS_CTX_CLIENT_ACCESSIBLE);
    VSS_ID snapshotSetId = GUID_NULL;
    VSS_ID snapshotId = GUID_NULL;
    if (FAILED(backup->StartSnapshotSet(&snapshotSetId)) ||
        FAILED(backup->AddToSnapshotSet((LPWSTR)volumeName.c_str(), GUID_NULL, &snapshotId))) {
        error = L"Unable to add volume to snapshot set";
        backup->Release();
        return false;
    }

    CComPtr<IVssAsync> async;
    if (FAILED(backup->DoSnapshotSet(&async)) || async == nullptr) {
        error = L"DoSnapshotSet failed";
        backup->Release();
        return false;
    }
    async->Wait();

    snapshotDeviceObject = volumeName;
    backup->Release();
    return true;
#else
    (void)volumeName;
    snapshotDeviceObject.clear();
    error = L"VSS is only available on Windows";
    return false;
#endif
}

bool VssProvider::DeleteSnapshot(const std::wstring& snapshotId, std::wstring& error) const {
#ifdef _WIN32
    (void)snapshotId;
    return true;
#else
    (void)snapshotId;
    error = L"VSS is only available on Windows";
    return false;
#endif
}

}  // namespace recovery
