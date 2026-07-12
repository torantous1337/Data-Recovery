#include "RecoveryEngine/Raid/RaidReconstructor.h"

#include <gtest/gtest.h>

using namespace recovery;

TEST(RaidReconstructorTests, MapsRaid0AcrossMembers) {
    RaidReconstructor raid(RaidLevel::Raid0, 4096, 3);
    auto map = raid.MapLogicalRead(0, 12288);

    ASSERT_EQ(map.size(), 3);
    EXPECT_EQ(map[0].memberIndex, 0u);
    EXPECT_EQ(map[1].memberIndex, 1u);
    EXPECT_EQ(map[2].memberIndex, 2u);
}
