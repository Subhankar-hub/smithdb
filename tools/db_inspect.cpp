// Prints basic physical information about a SmithDB database file.
//
// Usage: smithdb_inspect <database-file>

#include <exception>
#include <filesystem>
#include <iostream>

#include "smithdb/common/constants.hpp"
#include "smithdb/storage/disk_manager.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << (argc > 0 ? argv[0] : "smithdb_inspect") << " <database-file>\n";
        return 2;
    }

    const std::filesystem::path path = argv[1];
    // DiskManager creates missing files, so check first to keep the inspector read-only in intent.
    if (!std::filesystem::is_regular_file(path)) {
        std::cerr << "error: " << path.string() << " is not an existing regular file\n";
        return 1;
    }

    try {
        const smithdb::DiskManager disk(path);
        std::cout << "Database file: " << path.string() << '\n'
                  << "Page size: " << smithdb::PAGE_SIZE << '\n'
                  << "Number of pages: " << disk.page_count() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
