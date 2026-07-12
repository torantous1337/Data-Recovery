#include "RecoveryEngine/Parsers/ParserFactory.h"
#include "RecoveryEngine/IO/IDiskAccess.h"

#include <gtest/gtest.h>
#include <cstring>

using namespace recovery;

class FakeDiskAccess final : public IDiskAccess {
public:
    explicit FakeDiskAccess(std::vector<uint8_t> boot) : boot_(std::move(boot)) {
        geometry_.totalBytes = 4 * 1024 * 1024;
    }

    bool OpenReadOnly(const std::string&) override { return true; }
    void Close() override {}
    bool ReadSync(const ReadRequest& request, std::vector<uint8_t>& outBuffer, uint32_t& osError) override {
        if (request.offset != 0) {
            osError = 1;
            return false;
        }
        outBuffer = boot_;
        osError = 0;
        return true;
    }
    bool ReadAsync(const ReadRequest&, ReadCompletion) override { return false; }
    DiskGeometry Geometry() const override { return geometry_; }
    std::string Source() const override { return "fake"; }
    bool IsOpen() const override { return true; }

private:
    std::vector<uint8_t> boot_;
    DiskGeometry geometry_;
};

TEST(ParserFactoryTests, DetectsNtfsBootSector) {
    std::vector<uint8_t> boot(512, 0);
    std::memcpy(boot.data() + 3, "NTFS    ", 8);
    FakeDiskAccess disk(boot);

    auto parser = ParserFactory::CreateForDisk(disk);
    ASSERT_NE(parser, nullptr);
    EXPECT_EQ(parser->Type(), FileSystemType::NTFS);
}
