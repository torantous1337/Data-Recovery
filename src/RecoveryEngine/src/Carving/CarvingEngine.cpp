#include "RecoveryEngine/Carving/CarvingEngine.h"

#include <algorithm>
#include <cstring>
#include <unordered_set>

namespace recovery {

CarvingEngine::CarvingEngine(const SignatureDatabase* db) : db_(db) {
    std::vector<std::vector<uint8_t>> patterns;
    if (db_) {
        patterns.reserve(db_->Rules().size());
        for (const auto& rule : db_->Rules()) {
            patterns.push_back(rule.header);
        }
    }
    matcher_.Build(patterns);
}

bool CarvingEngine::ValidateCandidate(const FileSignatureRule& rule, const std::vector<uint8_t>& data) const {
    if (data.size() < rule.header.size()) {
        return false;
    }
    if (!std::equal(rule.header.begin(), rule.header.end(), data.begin())) {
        return false;
    }
    if (rule.requiresFooter) {
        if (data.size() < rule.footer.size()) {
            return false;
        }
        return std::search(data.begin(), data.end(), rule.footer.begin(), rule.footer.end()) != data.end();
    }
    return true;
}

std::vector<RecoverableFile> CarvingEngine::Carve(IDiskAccess& disk, uint64_t startOffset, uint64_t length, uint32_t chunkSize) {
    std::vector<RecoverableFile> out;
    if (!db_ || db_->Rules().empty()) {
        return out;
    }

    const uint32_t overlap = 4096;
    std::vector<uint8_t> carry;
    std::unordered_set<uint64_t> seenOffsets;

    for (uint64_t offset = startOffset; offset < startOffset + length; offset += chunkSize) {
        const auto currentSize = static_cast<uint32_t>(std::min<uint64_t>(chunkSize, startOffset + length - offset));
        std::vector<uint8_t> chunk;
        uint32_t err{};
        if (!disk.ReadSync({offset, currentSize}, chunk, err)) {
            continue;
        }

        std::vector<uint8_t> block;
        block.reserve(carry.size() + chunk.size());
        block.insert(block.end(), carry.begin(), carry.end());
        block.insert(block.end(), chunk.begin(), chunk.end());
        const auto base = offset - carry.size();

        for (const auto& hit : matcher_.Search(block, base)) {
            if (seenOffsets.find(hit.absoluteOffset) != seenOffsets.end()) {
                continue;
            }
            const auto& rule = db_->Rules()[hit.patternIndex];
            std::vector<uint8_t> sample;
            uint32_t readErr{};
            const auto probeSize = static_cast<uint32_t>(std::min<uint64_t>(rule.maxSize, 2 * 1024 * 1024));
            if (!disk.ReadSync({hit.absoluteOffset, probeSize}, sample, readErr)) {
                continue;
            }
            if (!ValidateCandidate(rule, sample)) {
                continue;
            }

            RecoverableFile file;
            file.id = static_cast<uint64_t>(out.size() + 1);
            file.path = "carved_" + std::to_string(file.id) + rule.extension;
            file.extension = rule.extension;
            file.size = sample.size();
            file.carved = true;
            file.deleted = true;
            file.sourceOffset = hit.absoluteOffset;
            file.health = FileHealth::Poor;
            out.push_back(std::move(file));
            seenOffsets.insert(hit.absoluteOffset);
        }

        if (block.size() > overlap) {
            carry.assign(block.end() - overlap, block.end());
        } else {
            carry = block;
        }
    }

    return out;
}

}  // namespace recovery
