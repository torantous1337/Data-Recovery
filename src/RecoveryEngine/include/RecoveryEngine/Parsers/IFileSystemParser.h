#pragma once

#include "RecoveryEngine/Core/Types.h"
#include "RecoveryEngine/IO/IDiskAccess.h"

#include <memory>
#include <string_view>
#include <vector>

namespace recovery {

struct ParserContext {
    IDiskAccess* disk{};
    ScanFilter filter;
};

class IFileSystemParser {
public:
    virtual ~IFileSystemParser() = default;
    virtual FileSystemType Type() const = 0;
    virtual bool Probe(IDiskAccess& disk) const = 0;
    virtual std::vector<RecoverableFile> EnumerateFiles(const ParserContext& context) = 0;
    virtual std::vector<RecoverableFile> RecoverDeleted(const ParserContext& context) = 0;
    virtual std::vector<uint64_t> ReadClusterChain(const ParserContext& context, uint64_t fileId) = 0;
    virtual std::string_view Name() const = 0;
};

}  // namespace recovery
