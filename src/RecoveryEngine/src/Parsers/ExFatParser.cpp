#include "RecoveryEngine/Parsers/IFileSystemParser.h"

#include <cstring>

namespace recovery {

class ExFatParser final : public IFileSystemParser {
public:
    FileSystemType Type() const override { return FileSystemType::ExFAT; }
    std::string_view Name() const override { return "exFAT"; }

    bool Probe(IDiskAccess& disk) const override {
        std::vector<uint8_t> boot;
        uint32_t err{};
        if (!disk.ReadSync({0, 512}, boot, err) || boot.size() < 11) {
            return false;
        }
        return std::memcmp(boot.data() + 3, "EXFAT   ", 8) == 0;
    }

    std::vector<RecoverableFile> EnumerateFiles(const ParserContext&) override { return {}; }
    std::vector<RecoverableFile> RecoverDeleted(const ParserContext&) override { return {}; }
    std::vector<uint64_t> ReadClusterChain(const ParserContext&, uint64_t) override { return {}; }
};

std::unique_ptr<IFileSystemParser> CreateExFatParser() {
    return std::make_unique<ExFatParser>();
}

}  // namespace recovery
