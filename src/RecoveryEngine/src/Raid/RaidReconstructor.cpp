#include "RecoveryEngine/Raid/RaidReconstructor.h"

#include <algorithm>

namespace recovery {

RaidReconstructor::RaidReconstructor(RaidLevel level, uint32_t stripeSize, size_t memberCount, size_t parityRotation)
    : level_(level), stripeSize_(stripeSize), memberCount_(memberCount), parityRotation_(parityRotation) {}

std::vector<LogicalReadMap> RaidReconstructor::MapLogicalRead(uint64_t logicalOffset, uint32_t length) const {
    std::vector<LogicalReadMap> maps;
    uint64_t cursor = logicalOffset;
    uint32_t remaining = length;

    while (remaining > 0) {
        const auto stripeIndex = cursor / stripeSize_;
        const auto inStripe = static_cast<uint32_t>(cursor % stripeSize_);
        const auto part = std::min<uint32_t>(remaining, stripeSize_ - inStripe);

        LogicalReadMap map{};
        if (level_ == RaidLevel::Raid0) {
            map.memberIndex = stripeIndex % memberCount_;
            map.memberOffset = (stripeIndex / memberCount_) * stripeSize_ + inStripe;
        } else if (level_ == RaidLevel::Raid1) {
            map.memberIndex = 0;
            map.memberOffset = stripeIndex * stripeSize_ + inStripe;
        } else {
            const auto dataDisks = memberCount_ - 1;
            const auto stripeSet = stripeIndex / dataDisks;
            const auto dataSlot = stripeIndex % dataDisks;
            const auto parityDisk = (parityRotation_ + stripeSet) % memberCount_;
            map.memberIndex = (dataSlot >= parityDisk) ? dataSlot + 1 : dataSlot;
            map.memberOffset = stripeSet * stripeSize_ + inStripe;
        }
        map.length = part;

        maps.push_back(map);
        cursor += part;
        remaining -= part;
    }

    return maps;
}

}  // namespace recovery
