#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <string>

#include "smithdb/common/status.hpp"
#include "smithdb/storage/buffer_pool.hpp"
#include "smithdb/storage/disk_manager.hpp"
#include "smithdb/storage/page.hpp"

namespace smithdb {
namespace {

class BufferPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        path_ = std::filesystem::temp_directory_path() /
                (std::string("smithdb_") + info->test_suite_name() + "_" + info->name() + ".db");
        std::filesystem::remove(path_);
    }

    void TearDown() override { std::filesystem::remove(path_); }

    std::filesystem::path path_;
};

TEST_F(BufferPoolTest, RejectsZeroSize) {
    DiskManager disk(path_);
    EXPECT_THROW(BufferPool(0, disk), DatabaseError);
}

TEST_F(BufferPoolTest, FetchingResidentPageReturnsSameFrame) {
    DiskManager disk(path_);
    const PageId id = disk.allocate_page();
    BufferPool pool(2, disk);

    Page* first = pool.fetch_page(id);
    Page* second = pool.fetch_page(id);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, second);
    pool.unpin_page(id, false);
    pool.unpin_page(id, false);
}

TEST_F(BufferPoolTest, FlushWritesDirtyPageToDisk) {
    DiskManager disk(path_);
    const PageId id = disk.allocate_page();
    BufferPool pool(2, disk);

    Page* page = pool.fetch_page(id);
    ASSERT_NE(page, nullptr);
    page->data()[10] = std::byte{0x5A};
    pool.unpin_page(id, true);
    pool.flush_page(id);

    Page on_disk;
    disk.read_page(id, on_disk);
    EXPECT_EQ(on_disk.data()[10], std::byte{0x5A});
}

TEST_F(BufferPoolTest, EvictionWritesBackDirtyPage) {
    DiskManager disk(path_);
    const PageId a = disk.allocate_page();
    const PageId b = disk.allocate_page();
    BufferPool pool(1, disk);

    Page* page = pool.fetch_page(a);
    ASSERT_NE(page, nullptr);
    page->data()[0] = std::byte{0x11};
    pool.unpin_page(a, true);

    ASSERT_NE(pool.fetch_page(b), nullptr);
    pool.unpin_page(b, false);

    Page on_disk;
    disk.read_page(a, on_disk);
    EXPECT_EQ(on_disk.data()[0], std::byte{0x11});
}

TEST_F(BufferPoolTest, ReturnsNullWhenAllFramesPinned) {
    DiskManager disk(path_);
    const PageId a = disk.allocate_page();
    const PageId b = disk.allocate_page();
    BufferPool pool(1, disk);

    ASSERT_NE(pool.fetch_page(a), nullptr);
    EXPECT_EQ(pool.fetch_page(b), nullptr);
    pool.unpin_page(a, false);
    EXPECT_NE(pool.fetch_page(b), nullptr);
}

TEST_F(BufferPoolTest, UnpinErrors) {
    DiskManager disk(path_);
    const PageId id = disk.allocate_page();
    BufferPool pool(1, disk);

    EXPECT_THROW(pool.unpin_page(id, false), DatabaseError);
    ASSERT_NE(pool.fetch_page(id), nullptr);
    pool.unpin_page(id, false);
    EXPECT_THROW(pool.unpin_page(id, false), DatabaseError);
}

}  // namespace
}  // namespace smithdb
