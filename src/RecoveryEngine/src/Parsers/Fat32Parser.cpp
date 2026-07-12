#include "RecoveryEngine/Parsers/IFileSystemParser.h"

#include <cstring>

namespace recovery {

class Fat32Parser final : public IFileSystemParser {
public:
    FileSystemType Type() const override { return FileSystemType::FAT32; }
    std::string_view Name() const override { return "FAT32"; }

    bool Probe(IDiskAccess& disk) const override {
        std::vector<uint8_t> boot;
        uint32_t err{};
        if (!disk.ReadSync({0, 512}, boot, err) || boot.size() < 90) {
            return false;
        }
        return std::memcmp(boot.data() + 82, "FAT32   ", 8) == 0;
    }

    std::vector<RecoverableFile> EnumerateFiles(const ParserContext&) override { return {}; }
    std::vector<RecoverableFile> RecoverDeleted(const ParserContext&) override { return {}; }
    std::vector<uint64_t> ReadClusterChain(const ParserContext&, uint64_t) override { return {}; }
};

std::unique_ptr<IFileSystemParser> CreateFat32Parser() {
    return std::make_unique<Fat32Parser>();
}

}  // namespace recovery
