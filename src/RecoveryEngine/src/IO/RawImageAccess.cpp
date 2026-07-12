#include "RecoveryEngine/IO/IDiskAccess.h"

#include <filesystem>
#include <fstream>
#include <future>
#include <mutex>

namespace recovery {

struct RawImageAccess::Impl {
    std::ifstream stream;
    std::mutex streamMutex;
    DiskGeometry geometry{};
    std::string source;
    bool open{};
};

RawImageAccess::RawImageAccess() : impl_(std::make_unique<Impl>()) {}
RawImageAccess::~RawImageAccess() { Close(); }

bool RawImageAccess::OpenReadOnly(const std::string& sourcePath) {
    Close();
    impl_->stream.open(sourcePath, std::ios::binary);
    if (!impl_->stream.is_open()) {
        return false;
    }
    impl_->geometry.totalBytes = std::filesystem::file_size(sourcePath);
    impl_->geometry.bytesPerSector = 512;
    impl_->source = sourcePath;
    impl_->open = true;
    return true;
}

void RawImageAccess::Close() {
    if (impl_->stream.is_open()) {
        impl_->stream.close();
    }
    impl_->open = false;
}

bool RawImageAccess::ReadSync(const ReadRequest& request, std::vector<uint8_t>& outBuffer, uint32_t& osError) {
    std::lock_guard<std::mutex> lock(impl_->streamMutex);
    if (!impl_->open) {
        osError = 1;
        return false;
    }
    if (request.offset + request.size > impl_->geometry.totalBytes) {
        osError = 2;
        return false;
    }

    outBuffer.resize(request.size);
    impl_->stream.clear();
    impl_->stream.seekg(static_cast<std::streamoff>(request.offset));
    impl_->stream.read(reinterpret_cast<char*>(outBuffer.data()), static_cast<std::streamsize>(request.size));
    if (impl_->stream.gcount() != static_cast<std::streamsize>(request.size)) {
        osError = 3;
        return false;
    }

    osError = 0;
    return true;
}

bool RawImageAccess::ReadAsync(const ReadRequest& request, ReadCompletion completion) {
    return std::async(std::launch::async, [this, request, completion = std::move(completion)] {
        std::vector<uint8_t> data;
        uint32_t error{};
        const bool ok = ReadSync(request, data, error);
        completion(ok, std::move(data), error);
    }).valid();
}

DiskGeometry RawImageAccess::Geometry() const { return impl_->geometry; }
std::string RawImageAccess::Source() const { return impl_->source; }
bool RawImageAccess::IsOpen() const { return impl_->open; }

}  // namespace recovery
