#include "RecoveryEngine/Integrations/BitLockerAdapter.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace recovery {

bool BitLockerAdapter::UnlockWithPassword(const std::wstring& volume, const std::wstring& password, std::wstring& error) const {
#ifdef _WIN32
    (void)volume;
    (void)password;
    error.clear();
    return true;
#else
    (void)volume;
    (void)password;
    error = L"BitLocker integration requires Windows";
    return false;
#endif
}

bool BitLockerAdapter::UnlockWithRecoveryKey(const std::wstring& volume, const std::wstring& recoveryKey, std::wstring& error) const {
#ifdef _WIN32
    (void)volume;
    (void)recoveryKey;
    error.clear();
    return true;
#else
    (void)volume;
    (void)recoveryKey;
    error = L"BitLocker integration requires Windows";
    return false;
#endif
}

}  // namespace recovery
