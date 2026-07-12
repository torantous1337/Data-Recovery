#include "RecoveryEngine/Carving/AhoCorasick.h"

#include <queue>

namespace recovery {

void AhoCorasick::Build(const std::vector<std::vector<uint8_t>>& patterns) {
    patterns_ = patterns;
    trie_.clear();
    trie_.push_back({});

    for (size_t i = 0; i < patterns.size(); ++i) {
        size_t node = 0;
        for (auto ch : patterns[i]) {
            auto it = trie_[node].next.find(ch);
            if (it == trie_[node].next.end()) {
                trie_[node].next[ch] = trie_.size();
                trie_.push_back({});
                node = trie_.size() - 1;
            } else {
                node = it->second;
            }
        }
        trie_[node].outputs.push_back(i);
    }

    std::queue<size_t> q;
    for (const auto& [ch, next] : trie_[0].next) {
        (void)ch;
        trie_[next].fail = 0;
        q.push(next);
    }

    while (!q.empty()) {
        const auto current = q.front();
        q.pop();

        for (const auto& [ch, child] : trie_[current].next) {
            q.push(child);
            size_t fail = trie_[current].fail;
            while (fail != 0 && trie_[fail].next.find(ch) == trie_[fail].next.end()) {
                fail = trie_[fail].fail;
            }
            if (const auto it = trie_[fail].next.find(ch); it != trie_[fail].next.end()) {
                trie_[child].fail = it->second;
            }
            const auto& failOutputs = trie_[trie_[child].fail].outputs;
            trie_[child].outputs.insert(trie_[child].outputs.end(), failOutputs.begin(), failOutputs.end());
        }
    }
}

std::vector<SignatureHit> AhoCorasick::Search(const std::vector<uint8_t>& data, uint64_t baseOffset) const {
    std::vector<SignatureHit> hits;
    size_t state = 0;

    for (size_t i = 0; i < data.size(); ++i) {
        const auto ch = data[i];

        while (state != 0 && trie_[state].next.find(ch) == trie_[state].next.end()) {
            state = trie_[state].fail;
        }

        if (const auto it = trie_[state].next.find(ch); it != trie_[state].next.end()) {
            state = it->second;
        }

        for (const auto patternIndex : trie_[state].outputs) {
            const auto patternSize = patterns_[patternIndex].size();
            if (i + 1 >= patternSize) {
                hits.push_back({patternIndex, baseOffset + i + 1 - patternSize});
            }
        }
    }

    return hits;
}

}  // namespace recovery
