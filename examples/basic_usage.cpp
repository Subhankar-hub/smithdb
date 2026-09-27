// Demonstrates the storage foundation that exists today: page-level file I/O through DiskManager.
// SmithDB does not yet have tables, records on disk, or SQL.

#include <cstddef>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

#include "smithdb/common/constants.hpp"
#include "smithdb/storage/disk_manager.hpp"
#include "smithdb/storage/page.hpp"

int main(int argc, char** argv) {
    const std::filesystem::path path = argc > 1 ? argv[1] : "smithdb_example.db";

    try {
        smithdb::DiskManager disk(path);
        std::cout << "Opened " << path.string() << " (" << disk.page_count() << " existing pages)\n";

        const smithdb::PageId page_id = disk.allocate_page();
        std::cout << "Allocated page " << page_id << '\n';

        constexpr std::string_view message = "Hello, SmithDB!";
        smithdb::Page page;
        for (std::size_t i = 0; i < message.size(); ++i) {
            page.data()[i] = static_cast<std::byte>(message[i]);
        }
        disk.write_page(page_id, page);
        disk.flush();
        std::cout << "Wrote \"" << message << "\" to page " << page_id << '\n';

        smithdb::Page read_back;
        disk.read_page(page_id, read_back);
        for (std::size_t i = 0; i < smithdb::PAGE_SIZE; ++i) {
            if (read_back.data()[i] != page.data()[i]) {
                std::cerr << "Verification failed at byte " << i << '\n';
                return 1;
            }
        }
        std::cout << "Read page " << page_id << " back and verified all " << smithdb::PAGE_SIZE
                  << " bytes\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
