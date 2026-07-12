#pragma once

#include <stdint.h>

#ifdef _WIN32
#define RECOVERY_API __declspec(dllexport)
#else
#define RECOVERY_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RecoveryHandle RecoveryHandle;

typedef struct RecoveryScanOptions {
    const char* source;
    const char* destination;
    const char* signatureDb;
    const char* sessionDb;
    int mode;
} RecoveryScanOptions;

RECOVERY_API RecoveryHandle* Recovery_Create();
RECOVERY_API void Recovery_Destroy(RecoveryHandle* handle);
RECOVERY_API int Recovery_StartScan(RecoveryHandle* handle, const RecoveryScanOptions* options);
RECOVERY_API int Recovery_Pause(RecoveryHandle* handle);
RECOVERY_API int Recovery_Resume(RecoveryHandle* handle);
RECOVERY_API int Recovery_Cancel(RecoveryHandle* handle);
RECOVERY_API int Recovery_SaveSession(RecoveryHandle* handle, const char* sessionId);
RECOVERY_API int Recovery_LoadSession(RecoveryHandle* handle, const char* sessionId);
RECOVERY_API int Recovery_GetLastError(RecoveryHandle* handle, char* buffer, uint32_t bufferLen);

#ifdef __cplusplus
}
#endif
