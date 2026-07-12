#include "RecoveryEngine/Imaging/ImagingEngine.h"

#include <fstream>

namespace recovery {

bool ImagingEngine::CreateRawImage(IDiskAccess& source,
                                   const std::string& destinationPath,
                                   ProgressCallback onProgress,
                                   std::atomic<bool>& cancelFlag) const {
    std::ofstream out(destinationPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    const auto total = source.Geometry().totalBytes;
    const uint32_t chunk = 1024 * 1024;
    uint64_t written{};

    while (written < total) {
        if (cancelFlag.load()) {
            return false;
        }
        const auto toRead = static_cast<uint32_t>(std::min<uint64_t>(chunk, total - written));
        std::vector<uint8_t> buffer;
        uint32_t err{};
        if (!source.ReadSync({written, toRead}, buffer, err)) {
            return false;
        }

        out.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        if (!out.good()) {
            return false;
        }

        written += buffer.size();
        if (onProgress) {
            onProgress(written, total);
        }
    }

    return true;
}

}  // namespace recovery
