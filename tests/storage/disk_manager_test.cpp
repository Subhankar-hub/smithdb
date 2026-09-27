#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>

#include "smithdb/common/constants.hpp"
#include "smithdb/common/status.hpp"
#include "smithdb/storage/disk_manager.hpp"
#include "smithdb/storage/page.hpp"

namespace smithdb {
namespace {

class DiskManagerTest : public ::testing::Test {
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

void fill_pattern(Page& page, unsigned seed) {
    for (std::size_t i = 0; i < PAGE_SIZE; ++i) {
        page.data()[i] = static_cast<std::byte>((i + seed) % 251);
    }
}

bool same_bytes(const Page& a, const Page& b) {
    return std::ranges::equal(std::span<const std::byte>(a.data(), PAGE_SIZE),
                              std::span<const std::byte>(b.data(), PAGE_SIZE));
}

TEST_F(DiskManagerTest, CreatesEmptyFile) {
    DiskManager disk(path_);
    EXPECT_TRUE(std::filesystem::exists(path_));
    EXPECT_EQ(disk.page_count(), 0U);
}

TEST_F(DiskManagerTest, AllocatesSequentialPageIdsAndGrowsFile) {
    DiskManager disk(path_);
    EXPECT_EQ(disk.allocate_page(), 0U);
    EXPECT_EQ(disk.allocate_page(), 1U);
    EXPECT_EQ(disk.allocate_page(), 2U);
    EXPECT_EQ(disk.page_count(), 3U);
    disk.flush();
    EXPECT_EQ(std::filesystem::file_size(path_), 3 * PAGE_SIZE);
}

TEST_F(DiskManagerTest, AllocatedPageIsZeroFilled) {
    DiskManager disk(path_);
    const PageId id = disk.allocate_page();
    Page page;
    fill_pattern(page, 1);
    disk.read_page(id, page);
    EXPECT_TRUE(same_bytes(page, Page{}));
}

TEST_F(DiskManagerTest, WriteThenReadReturnsSameBytes) {
    DiskManager disk(path_);
    const PageId first = disk.allocate_page();
    const PageId second = disk.allocate_page();

    Page a;
    Page b;
    fill_pattern(a, 1);
    fill_pattern(b, 2);
    disk.write_page(first, a);
    disk.write_page(second, b);

    Page out;
    disk.read_page(first, out);
    EXPECT_TRUE(same_bytes(out, a));
    disk.read_page(second, out);
    EXPECT_TRUE(same_bytes(out, b));
}

TEST_F(DiskManagerTest, DataPersistsAcrossReopen) {
    Page written;
    fill_pattern(written, 42);
    PageId id = 0;
    {
        DiskManager disk(path_);
        disk.allocate_page();
        id = disk.allocate_page();
        disk.write_page(id, written);
        disk.flush();
    }

    DiskManager reopened(path_);
    EXPECT_EQ(reopened.page_count(), 2U);
    Page read;
    reopened.read_page(id, read);
    EXPECT_TRUE(same_bytes(read, written));
}

TEST_F(DiskManagerTest, AccessingUnallocatedPageThrows) {
    DiskManager disk(path_);
    disk.allocate_page();
    Page page;
    try {
        disk.read_page(1, page);
        FAIL() << "expected DatabaseError";
    } catch (const DatabaseError& e) {
        EXPECT_EQ(e.code(), ErrorCode::OutOfRange);
    }
    EXPECT_THROW(disk.write_page(1, page), DatabaseError);
}

TEST_F(DiskManagerTest, RejectsFileWithPartialPage) {
    {
        std::ofstream out(path_, std::ios::binary);
        out << "not a whole page";
    }
    try {
        DiskManager disk(path_);
        FAIL() << "expected DatabaseError";
    } catch (const DatabaseError& e) {
        EXPECT_EQ(e.code(), ErrorCode::Corruption);
    }
}

}  // namespace
}  // namespace smithdb
