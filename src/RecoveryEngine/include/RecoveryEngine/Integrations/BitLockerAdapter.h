#pragma once

#include <string>

namespace recovery {

class BitLockerAdapter {
public:
    bool UnlockWithPassword(const std::wstring& volume, const std::wstring& password, std::wstring& error) const;
    bool UnlockWithRecoveryKey(const std::wstring& volume, const std::wstring& recoveryKey, std::wstring& error) const;
};

}  // namespace recovery
