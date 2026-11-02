#include "render/TextureStorageInflator.h"

#include <fstream>
#include <unordered_set>

#include "debug/Logger.h"

namespace {
    const char *component = "TEX INFL";
    constexpr size_t MAX_ID_LIMIT = 4095;
}

namespace render {
    void TextureStorageInflator::inflateStorage(TextureStorage &storage, const std::string &mapping_file) {
        namespace fs = std::filesystem;
        auto fail = [&](const std::string& msg) {
            logging::error(component, 2, msg, "");
            throw std::runtime_error(msg);
        };

        fs::path path = fs::current_path() / mapping_file;
        std::ifstream stream(path);
        if (!stream) {
            fail(std::format("cannot open texture mapping file: \"{}\"", mapping_file));
        }


        std::vector<TextureStorage::TextureEntry> entries;
        size_t id;
        std::string filename;
        size_t max_id = 0;
        std::unordered_set<size_t> seen_ids;
        //TODO make adequate parser
        while (stream >> id >> filename) {
            if (!seen_ids.insert(id).second) {
                fail(std::format("duplicate texture id {}", id));
            }
            if (max_id > MAX_ID_LIMIT) {
                fail(std::format("texture id {} exceeds limit {}", id, MAX_ID_LIMIT));
            }

            entries.emplace_back(filename, id);
            max_id = std::max(max_id, id);

        }

        logging::info(
            component,
            std::format("found entries up to {} id in mapping file: \"{}\"", max_id, mapping_file)
            );
        try {
            storage.resize(max_id + 1);
            storage.addFromEntries(entries);
        } catch (const std::exception &ex) {
           fail(std::format("texture entries processing error: {}", ex.what()));
        }
    }
}
