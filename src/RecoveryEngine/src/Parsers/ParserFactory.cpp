#include "RecoveryEngine/Parsers/ParserFactory.h"

namespace recovery {

std::unique_ptr<IFileSystemParser> CreateNtfsParser();
std::unique_ptr<IFileSystemParser> CreateFat32Parser();
std::unique_ptr<IFileSystemParser> CreateExFatParser();
std::unique_ptr<IFileSystemParser> CreateRefsParser();

std::vector<std::unique_ptr<IFileSystemParser>> ParserFactory::CreateBuiltins() {
    std::vector<std::unique_ptr<IFileSystemParser>> parsers;
    parsers.emplace_back(CreateNtfsParser());
    parsers.emplace_back(CreateFat32Parser());
    parsers.emplace_back(CreateExFatParser());
    parsers.emplace_back(CreateRefsParser());
    return parsers;
}

std::unique_ptr<IFileSystemParser> ParserFactory::CreateForDisk(IDiskAccess& disk, FileSystemType overrideType) {
    auto builtins = CreateBuiltins();
    if (overrideType != FileSystemType::Unknown) {
        for (auto& parser : builtins) {
            if (parser->Type() == overrideType) {
                return std::move(parser);
            }
        }
        return {};
    }

    for (auto& parser : builtins) {
        if (parser->Probe(disk)) {
            return std::move(parser);
        }
    }
    return {};
}

}  // namespace recovery
