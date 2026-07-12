#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace recovery {

struct SignatureHit {
    size_t patternIndex{};
    uint64_t absoluteOffset{};
};

class AhoCorasick {
public:
    void Build(const std::vector<std::vector<uint8_t>>& patterns);
    std::vector<SignatureHit> Search(const std::vector<uint8_t>& data, uint64_t baseOffset) const;

private:
    struct Node {
        std::unordered_map<uint8_t, size_t> next;
        size_t fail{};
        std::vector<size_t> outputs;
    };

    std::vector<std::vector<uint8_t>> patterns_;
    std::vector<Node> trie_;
};

}  // namespace recovery
