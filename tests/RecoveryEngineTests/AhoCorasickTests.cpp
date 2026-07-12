#include "RecoveryEngine/Carving/AhoCorasick.h"

#include <gtest/gtest.h>

using namespace recovery;

TEST(AhoCorasickTests, FindsOverlappingSignatures) {
    AhoCorasick ac;
    ac.Build({{0x41, 0x42}, {0x42, 0x43}, {0x41, 0x42, 0x43}});

    std::vector<uint8_t> input{0x41, 0x42, 0x43};
    auto hits = ac.Search(input, 1000);

    EXPECT_EQ(hits.size(), 3);
}
