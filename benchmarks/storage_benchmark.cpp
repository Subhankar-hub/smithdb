// Rough timing of sequential page writes and reads through DiskManager.
//
// Usage: smithdb_storage_benchmark [page-count]
//
// This measures buffered stream I/O (no fsync), so results reflect the OS page cache rather than
// the physical device.

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#include "smithdb/common/constants.hpp"
#include "smithdb/storage/disk_manager.hpp"
#include "smithdb/storage/page.hpp"

namespace {

using Clock = std::chrono::steady_clock;

void report(const char* label, smithdb::PageId pages, Clock::duration elapsed) {
    const double seconds = std::chrono::duration<double>(elapsed).count();
    const double mib = static_cast<double>(pages) * static_cast<double>(smithdb::PAGE_SIZE) /
                       (1024.0 * 1024.0);
    std::cout << label << ": " << pages << " pages in " << seconds * 1000.0 << " ms ("
              << (seconds > 0.0 ? mib / seconds : 0.0) << " MiB/s)\n";
}

}  // namespace

int main(int argc, char** argv) {
    smithdb::PageId pages = 10000;
    if (argc > 1) {
        pages = static_cast<smithdb::PageId>(std::stoul(argv[1]));
    }

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "smithdb_storage_benchmark.db";
    std::filesystem::remove(path);

    try {
        {
            smithdb::DiskManager disk(path);
            smithdb::Page page;

            auto start = Clock::now();
            for (smithdb::PageId i = 0; i < pages; ++i) {
                disk.allocate_page();
                page.data()[0] = static_cast<std::byte>(i & 0xFFU);
                disk.write_page(i, page);
            }
            disk.flush();
            report("allocate+write", pages, Clock::now() - start);

            start = Clock::now();
            for (smithdb::PageId i = 0; i < pages; ++i) {
                disk.read_page(i, page);
            }
            report("read", pages, Clock::now() - start);
        }
        std::filesystem::remove(path);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        std::filesystem::remove(path);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
