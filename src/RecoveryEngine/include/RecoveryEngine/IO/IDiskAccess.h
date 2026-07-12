#pragma once

#include "RecoveryEngine/Core/Types.h"

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace recovery {

class IDiskAccess {
public:
    using ReadCompletion = std::function<void(bool, std::vector<uint8_t>, uint32_t)>;

    virtual ~IDiskAccess() = default;
    virtual bool OpenReadOnly(const std::string& sourcePath) = 0;
    virtual void Close() = 0;
    virtual bool ReadSync(const ReadRequest& request, std::vector<uint8_t>& outBuffer, uint32_t& osError) = 0;
    virtual bool ReadAsync(const ReadRequest& request, ReadCompletion completion) = 0;
    virtual DiskGeometry Geometry() const = 0;
    virtual std::string Source() const = 0;
    virtual bool IsOpen() const = 0;
};

class PhysicalDriveAccess final : public IDiskAccess {
public:
    PhysicalDriveAccess();
    ~PhysicalDriveAccess() override;

    bool OpenReadOnly(const std::string& sourcePath) override;
    void Close() override;
    bool ReadSync(const ReadRequest& request, std::vector<uint8_t>& outBuffer, uint32_t& osError) override;
    bool ReadAsync(const ReadRequest& request, ReadCompletion completion) override;
    DiskGeometry Geometry() const override;
    std::string Source() const override;
    bool IsOpen() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class RawImageAccess final : public IDiskAccess {
public:
    RawImageAccess();
    ~RawImageAccess() override;

    bool OpenReadOnly(const std::string& sourcePath) override;
    void Close() override;
    bool ReadSync(const ReadRequest& request, std::vector<uint8_t>& outBuffer, uint32_t& osError) override;
    bool ReadAsync(const ReadRequest& request, ReadCompletion completion) override;
    DiskGeometry Geometry() const override;
    std::string Source() const override;
    bool IsOpen() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

bool EnableBackupPrivilege();

}  // namespace recovery
