#pragma once

#include "RecoveryEngine/IO/IDiskAccess.h"

#include <atomic>
#include <functional>
#include <string>

namespace recovery {

class ImagingEngine {
public:
    using ProgressCallback = std::function<void(uint64_t, uint64_t)>;

    bool CreateRawImage(IDiskAccess& source, const std::string& destinationPath, ProgressCallback onProgress, std::atomic<bool>& cancelFlag) const;
};

}  // namespace recovery
