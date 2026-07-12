#include "RecoveryEngine/IO/IDiskAccess.h"

#include <future>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace recovery {

struct PhysicalDriveAccess::Impl {
#ifdef _WIN32
    HANDLE handle{INVALID_HANDLE_VALUE};
#endif
    DiskGeometry geometry{};
    std::string source;
    bool open{};
};

PhysicalDriveAccess::PhysicalDriveAccess() : impl_(std::make_unique<Impl>()) {}
PhysicalDriveAccess::~PhysicalDriveAccess() { Close(); }

bool EnableBackupPrivilege() {
#ifdef _WIN32
    HANDLE token{};
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        return false;
    }
    TOKEN_PRIVILEGES tp{};
    LUID luid{};
    if (!LookupPrivilegeValueW(nullptr, SE_BACKUP_NAME, &luid)) {
        CloseHandle(token);
        return false;
    }
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    const BOOL adjusted = AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), nullptr, nullptr);
    const auto err = GetLastError();
    CloseHandle(token);
    return adjusted == TRUE && err == ERROR_SUCCESS;
#else
    return true;
#endif
}

bool PhysicalDriveAccess::OpenReadOnly(const std::string& sourcePath) {
    Close();
#ifdef _WIN32
    EnableBackupPrivilege();
    impl_->handle = CreateFileA(sourcePath.c_str(), GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                nullptr, OPEN_EXISTING,
                                FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                                nullptr);
    if (impl_->handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    GET_LENGTH_INFORMATION len{};
    DWORD bytesReturned{};
    if (!DeviceIoControl(impl_->handle, IOCTL_DISK_GET_LENGTH_INFO, nullptr, 0,
                         &len, sizeof(len), &bytesReturned, nullptr)) {
        Close();
        return false;
    }
    DISK_GEOMETRY_EX geometry{};
    if (!DeviceIoControl(impl_->handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0,
                         &geometry, sizeof(geometry), &bytesReturned, nullptr)) {
        Close();
        return false;
    }
    impl_->geometry.totalBytes = static_cast<uint64_t>(len.Length.QuadPart);
    impl_->geometry.bytesPerSector = geometry.Geometry.BytesPerSector;
#else
    (void)sourcePath;
    return false;
#endif
    impl_->source = sourcePath;
    impl_->open = true;
    return true;
}

void PhysicalDriveAccess::Close() {
#ifdef _WIN32
    if (impl_->handle != INVALID_HANDLE_VALUE) {
        CloseHandle(impl_->handle);
        impl_->handle = INVALID_HANDLE_VALUE;
    }
#endif
    impl_->open = false;
}

bool PhysicalDriveAccess::ReadSync(const ReadRequest& request, std::vector<uint8_t>& outBuffer, uint32_t& osError) {
    if (!impl_->open) {
        osError = 1;
        return false;
    }
#ifdef _WIN32
    const uint32_t alignment = impl_->geometry.bytesPerSector;
    uint64_t alignedOffset = request.offset - (request.offset % alignment);
    uint32_t shift = static_cast<uint32_t>(request.offset - alignedOffset);
    uint32_t alignedSize = request.size + shift;
    if (alignedSize % alignment != 0) {
        alignedSize += (alignment - (alignedSize % alignment));
    }

    outBuffer.assign(alignedSize, 0);
    OVERLAPPED ov{};
    ov.Offset = static_cast<DWORD>(alignedOffset & 0xFFFFFFFF);
    ov.OffsetHigh = static_cast<DWORD>(alignedOffset >> 32);

    DWORD readBytes{};
    const BOOL ok = ReadFile(impl_->handle, outBuffer.data(), alignedSize, &readBytes, &ov);
    if (!ok && GetLastError() == ERROR_IO_PENDING) {
        if (!GetOverlappedResult(impl_->handle, &ov, &readBytes, TRUE)) {
            osError = GetLastError();
            return false;
        }
    } else if (!ok) {
        osError = GetLastError();
        return false;
    }

    if (shift > 0 || request.size < outBuffer.size()) {
        std::vector<uint8_t> trimmed(outBuffer.begin() + shift, outBuffer.begin() + shift + request.size);
        outBuffer.swap(trimmed);
    }
    osError = 0;
    return true;
#else
    (void)request;
    (void)outBuffer;
    osError = 1;
    return false;
#endif
}

bool PhysicalDriveAccess::ReadAsync(const ReadRequest& request, ReadCompletion completion) {
    return std::async(std::launch::async, [this, request, completion = std::move(completion)] {
        std::vector<uint8_t> data;
        uint32_t error{};
        const bool ok = ReadSync(request, data, error);
        completion(ok, std::move(data), error);
    }).valid();
}

DiskGeometry PhysicalDriveAccess::Geometry() const { return impl_->geometry; }
std::string PhysicalDriveAccess::Source() const { return impl_->source; }
bool PhysicalDriveAccess::IsOpen() const { return impl_->open; }

}  // namespace recovery
