#include "RecoveryEngine/Carving/SignatureDatabase.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace recovery {

using json = nlohmann::json;

static std::vector<uint8_t> ParseHex(const std::string& hex) {
    std::vector<uint8_t> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        const auto byte = static_cast<uint8_t>(std::stoi(hex.substr(i, 2), nullptr, 16));
        out.push_back(byte);
    }
    return out;
}

bool SignatureDatabase::LoadFromJson(const std::string& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        return false;
    }

    json j;
    in >> j;
    rules_.clear();

    for (const auto& item : j.at("signatures")) {
        FileSignatureRule rule;
        rule.type = item.at("type").get<std::string>();
        rule.extension = item.at("extension").get<std::string>();
        rule.header = ParseHex(item.at("header_hex").get<std::string>());
        if (item.contains("footer_hex")) {
            rule.footer = ParseHex(item.at("footer_hex").get<std::string>());
            rule.requiresFooter = !rule.footer.empty();
        }
        if (item.contains("max_size")) {
            rule.maxSize = item.at("max_size").get<uint64_t>();
        }
        rules_.push_back(std::move(rule));
    }

    return !rules_.empty();
}

const std::vector<FileSignatureRule>& SignatureDatabase::Rules() const {
    return rules_;
}

}  // namespace recovery
