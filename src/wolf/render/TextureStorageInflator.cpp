#include "render/TextureStorageInflator.h"

#include <algorithm>
#include <charconv>
#include <format>
#include <fstream>
#include <numeric>
#include <unordered_set>
#include <utility>

#include "debug/Logger.h"

namespace {
    constexpr const char *component = "TXINFL";
    constexpr size_t MAX_ID_LIMIT = 4095;

    void fail(const std::string &msg) {
        logging::error(component, 2, msg, "");
        throw std::runtime_error(std::format("{} error", component));
    };

    std::string_view trim(std::string_view s) {
        constexpr auto to_skip = " \t\r";
        s.remove_prefix(std::min(s.find_first_not_of(to_skip), s.size()));
        s.remove_suffix(s.size() - (s.find_last_not_of(to_skip) + 1));
        return s;
    }
}

namespace render {

    std::vector<TextureStorage::TextureEntry> TextureStorageInflator::parseMapping(
        std::istream &stream,
        const fs::path &base_dir
        ) {
        std::vector<TextureStorage::TextureEntry> entries;
        std::string buf;
        for (size_t line = 1; std::getline(stream, buf); ++line) {
            std::string_view view = trim(buf);
            if (view.empty() || view.starts_with('#')) continue;

            const char *end = view.data() + view.size();
            size_t id = 0;
            auto [ptr, error_code] = std::from_chars(view.data(), end, id);
            if (error_code != std::errc{}) fail(std::format("mapping file line {}: invalid id", line));

            const std::string_view path = trim({ptr, static_cast<size_t>(end - ptr)});
            if (path.empty()) fail(std::format("mapping line {}: missing file path", line));

            entries.emplace_back((base_dir / fs::path(path)).string(), id);
        }

        return entries;
    }

    TextureStorageInflator::TextureStorageInflator(fs::path mapping_file_path)
    : mapping_file_path_(std::move(mapping_file_path)) {}

    void TextureStorageInflator::inflate(TextureStorage &storage) const {
        std::ifstream stream(mapping_file_path_);
        if (!stream) {
            fail(std::format("cannot open texture mapping file: \"{}\"", mapping_file_path_.string()));
        }

        const auto entries =
            parseMapping(stream, mapping_file_path_.parent_path());

        std::unordered_set<size_t> seen_ids;
        size_t size = 0;
        for (const auto&[filepath, id]: entries) {
            if (id > MAX_ID_LIMIT) {
                fail(std::format("texture id {} exceeds limit of {}", id, MAX_ID_LIMIT));
            }

            if (!seen_ids.insert(id).second) { fail(std::format("duplicate texture id {}", id)); }
            size = std::max(size, id + 1);
        }

        try {
            storage.reset(size);
            storage.addFromEntries(entries);
        } catch (const std::exception &ex) {
            fail(std::format("texture entries processing error: {}", ex.what()));
        }
    }
}
