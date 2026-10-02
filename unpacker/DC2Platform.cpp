#include "DC2Platform.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace dc2 {

bool readContainer(const std::string& filename, std::vector<ContainerEntry>& entries) {
    entries.clear();
    std::ifstream input(filename, std::ios::binary | std::ios::ate);
    if (!input) return false;
    const std::uint64_t size = static_cast<std::uint64_t>(input.tellg());
    if (size < 0x800) return false;
    input.seekg(0);
    unsigned char table[0x800];
    input.read(reinterpret_cast<char*>(table), sizeof(table));
    std::uint64_t offset = 0x800;
    for (std::size_t o = 0; o < sizeof(table); o += 0x20) {
        if (std::memcmp(table + o, "dummy header", 12) == 0) break;
        ContainerEntry e;
        std::memcpy(&e.type, table + o, 4);
        std::memcpy(&e.size, table + o + 4, 4);
        std::memcpy(&e.address, table + o + 8, 4);
        std::memcpy(&e.extra, table + o + 12, 4);
        if (e.type > 0x10 || e.size > size || offset + e.size > size) return false;
        e.offset = offset;
        entries.push_back(e);
        offset += (std::uint64_t(e.size) + 0x7FF) & ~std::uint64_t(0x7FF);
    }
    // A table of all-zero entries (ITEM.DAT, FILE.DAT ...) is image data, not a container.
    return std::any_of(entries.begin(), entries.end(), [](const ContainerEntry& e) { return e.size != 0; });
}

namespace {
// Room backgrounds (ST*.DBS): u32 0, u32 count, u32 load address, ...; the address is in
// PlayStation RAM (0x80xxxxxx) on that version.
GameVersion detectBackgrounds(const std::string& filename) {
    std::ifstream input(filename, std::ios::binary);
    std::uint32_t words[3] = {};
    input.read(reinterpret_cast<char*>(words), sizeof(words));
    if (!input || words[0] != 0 || words[2] == 0) return GameVersion::Unknown;
    return (words[2] & 0x80000000u) ? GameVersion::PlayStation : GameVersion::PC;
}

bool payloadStartsWith(const std::string& filename, const ContainerEntry& e, const char* magic) {
    std::ifstream input(filename, std::ios::binary);
    char head[4] = {};
    input.seekg(static_cast<std::streamoff>(e.offset));
    input.read(head, 4);
    return input && std::memcmp(head, magic, 4) == 0;
}
}

GameVersion detectVersion(const std::string& filename) {
    std::string ext = std::filesystem::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::toupper(c); });
    if (ext == ".DBS") return detectBackgrounds(filename);

    std::vector<ContainerEntry> entries;
    if (!readContainer(filename, entries)) return GameVersion::Unknown;
    for (const auto& e : entries) {
        const bool ramAddress = (e.address & 0x80000000u) != 0;
        if (e.type == 8 || (ramAddress && (e.type == 0 || e.type == 5 || e.type == 7)))
            return GameVersion::PlayStation;
        if (e.type == 3 && e.size >= 4 && payloadStartsWith(filename, e, "Gian"))
            return GameVersion::PlayStation;
        if (!ramAddress && e.address && (e.type == 5 || e.type == 6 || (e.type == 0 && e.size)))
            return GameVersion::PC;
    }
    return GameVersion::Unknown;
}

const char* versionName(GameVersion version) {
    switch (version) {
    case GameVersion::PC: return "PC";
    case GameVersion::PlayStation: return "PlayStation";
    default: return "either (same format on both)";
    }
}

}
