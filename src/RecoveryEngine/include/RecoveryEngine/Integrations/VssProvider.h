#pragma once

#include <string>

namespace recovery {

class VssProvider {
public:
    bool CreateSnapshot(const std::wstring& volumeName, std::wstring& snapshotDeviceObject, std::wstring& error) const;
    bool DeleteSnapshot(const std::wstring& snapshotId, std::wstring& error) const;
};

}  // namespace recovery
