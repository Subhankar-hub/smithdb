#include "smithdb/storage/disk_manager.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <system_error>

#include "smithdb/common/constants.hpp"
#include "smithdb/common/status.hpp"

namespace smithdb {

namespace {

constexpr auto PAGE_BYTES = static_cast<std::streamsize>(PAGE_SIZE);

std::streamoff page_offset(PageId page_id) {
    return static_cast<std::streamoff>(page_id) * static_cast<std::streamoff>(PAGE_SIZE);
}

}  // namespace

DiskManager::DiskManager(const std::filesystem::path& path) : path_(path) {
    if (!std::filesystem::exists(path_)) {
        // std::fstream cannot open a missing file for reading and writing, so create it first.
        std::ofstream create(path_, std::ios::binary);
        if (!create) {
            throw DatabaseError(ErrorCode::IoError,
                                "cannot create database file: " + path_.string());
        }
    }

    file_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_) {
        throw DatabaseError(ErrorCode::IoError, "cannot open database file: " + path_.string());
    }

    std::error_code ec;
    const std::uintmax_t file_size = std::filesystem::file_size(path_, ec);
    if (ec) {
        throw DatabaseError(ErrorCode::IoError,
                            "cannot determine size of database file: " + path_.string());
    }
    if (file_size % PAGE_SIZE != 0) {
        throw DatabaseError(ErrorCode::Corruption,
                            "database file size " + std::to_string(file_size) +
                                " is not a multiple of the page size");
    }
    const std::uintmax_t pages = file_size / PAGE_SIZE;
    if (pages > std::numeric_limits<PageId>::max()) {
        throw DatabaseError(ErrorCode::Corruption, "database file has too many pages");
    }
    page_count_ = static_cast<PageId>(pages);
}

DiskManager::~DiskManager() { file_.flush(); }

PageId DiskManager::allocate_page() {
    if (page_count_ == std::numeric_limits<PageId>::max()) {
        throw DatabaseError(ErrorCode::OutOfRange, "page ID space exhausted");
    }
    const PageId page_id = page_count_;
    write_at(page_id, Page{});
    ++page_count_;
    return page_id;
}

void DiskManager::read_page(PageId page_id, Page& page) {
    check_page_exists(page_id);
    file_.seekg(page_offset(page_id));
    file_.read(reinterpret_cast<char*>(page.data()), PAGE_BYTES);
    if (!file_ || file_.gcount() != PAGE_BYTES) {
        file_.clear();
        throw DatabaseError(ErrorCode::IoError, "failed to read page " + std::to_string(page_id));
    }
}

void DiskManager::write_page(PageId page_id, const Page& page) {
    check_page_exists(page_id);
    write_at(page_id, page);
}

void DiskManager::flush() {
    file_.flush();
    if (!file_) {
        file_.clear();
        throw DatabaseError(ErrorCode::IoError, "failed to flush database file");
    }
}

void DiskManager::check_page_exists(PageId page_id) const {
    if (page_id >= page_count_) {
        throw DatabaseError(ErrorCode::OutOfRange,
                            "page " + std::to_string(page_id) + " does not exist (page count " +
                                std::to_string(page_count_) + ")");
    }
}

void DiskManager::write_at(PageId page_id, const Page& page) {
    file_.seekp(page_offset(page_id));
    file_.write(reinterpret_cast<const char*>(page.data()), PAGE_BYTES);
    if (!file_) {
        file_.clear();
        throw DatabaseError(ErrorCode::IoError, "failed to write page " + std::to_string(page_id));
    }
}

}  // namespace smithdb
