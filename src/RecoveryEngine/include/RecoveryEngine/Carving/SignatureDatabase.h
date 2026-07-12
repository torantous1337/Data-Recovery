#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace recovery {

struct FileSignatureRule {
    std::string type;
    std::string extension;
    std::vector<uint8_t> header;
    std::vector<uint8_t> footer;
    uint64_t maxSize{64ULL * 1024 * 1024};
    bool requiresFooter{false};
};

class SignatureDatabase {
public:
    bool LoadFromJson(const std::string& filePath);
    const std::vector<FileSignatureRule>& Rules() const;

private:
    std::vector<FileSignatureRule> rules_;
};

}  // namespace recovery
