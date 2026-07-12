#pragma once

#include "RecoveryEngine/Carving/AhoCorasick.h"
#include "RecoveryEngine/Carving/SignatureDatabase.h"
#include "RecoveryEngine/Core/Types.h"
#include "RecoveryEngine/IO/IDiskAccess.h"

#include <unordered_set>

namespace recovery {

class CarvingEngine {
public:
    explicit CarvingEngine(const SignatureDatabase* db);
    std::vector<RecoverableFile> Carve(IDiskAccess& disk, uint64_t startOffset, uint64_t length, uint32_t chunkSize = 1024 * 1024);

private:
    bool ValidateCandidate(const FileSignatureRule& rule, const std::vector<uint8_t>& data) const;

    const SignatureDatabase* db_;
    AhoCorasick matcher_;
};

}  // namespace recovery
