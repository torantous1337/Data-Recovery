#pragma once

#include <cstdint>
#include <vector>

namespace recovery {

enum class RaidLevel : uint8_t {
    Raid0,
    Raid1,
    Raid5
};

struct LogicalReadMap {
    size_t memberIndex{};
    uint64_t memberOffset{};
    uint32_t length{};
};

class RaidReconstructor {
public:
    RaidReconstructor(RaidLevel level, uint32_t stripeSize, size_t memberCount, size_t parityRotation = 0);

    std::vector<LogicalReadMap> MapLogicalRead(uint64_t logicalOffset, uint32_t length) const;

private:
    RaidLevel level_;
    uint32_t stripeSize_;
    size_t memberCount_;
    size_t parityRotation_;
};

}  // namespace recovery
