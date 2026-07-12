#include "RecoveryEngine/Parsers/IFileSystemParser.h"

#include <cstring>

namespace recovery {

class RefsParser final : public IFileSystemParser {
public:
    FileSystemType Type() const override { return FileSystemType::ReFS; }
    std::string_view Name() const override { return "ReFS"; }

    bool Probe(IDiskAccess& disk) const override {
        std::vector<uint8_t> boot;
        uint32_t err{};
        if (!disk.ReadSync({0, 4096}, boot, err) || boot.size() < 11) {
            return false;
        }
        return std::memcmp(boot.data() + 3, "ReFS", 4) == 0;
    }

    std::vector<RecoverableFile> EnumerateFiles(const ParserContext&) override { return {}; }
    std::vector<RecoverableFile> RecoverDeleted(const ParserContext&) override { return {}; }
    std::vector<uint64_t> ReadClusterChain(const ParserContext&, uint64_t) override { return {}; }
};

std::unique_ptr<IFileSystemParser> CreateRefsParser() {
    return std::make_unique<RefsParser>();
}

}  // namespace recovery
