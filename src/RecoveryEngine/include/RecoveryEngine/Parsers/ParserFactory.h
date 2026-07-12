#pragma once

#include "RecoveryEngine/Parsers/IFileSystemParser.h"

#include <memory>
#include <vector>

namespace recovery {

class ParserFactory {
public:
    static std::vector<std::unique_ptr<IFileSystemParser>> CreateBuiltins();
    static std::unique_ptr<IFileSystemParser> CreateForDisk(IDiskAccess& disk, FileSystemType overrideType = FileSystemType::Unknown);
};

}  // namespace recovery
