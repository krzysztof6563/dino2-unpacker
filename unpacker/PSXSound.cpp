#include "PSXSound.h"
#include "DC2Platform.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace dc2 {
namespace {

constexpr int SAMPLE_RATE = 22050;     // every PC conversion of these samples uses this rate

// SPU ADPCM: 16-byte blocks of a shift/filter byte, a flag byte and 28 4-bit samples.
std::vector<std::int16_t> decodeAdpcm(const unsigned char* data, std::size_t blocks) {
    static const int filters[5][2] = {{0, 0}, {60, 0}, {115, -52}, {98, -55}, {122, -60}};
    std::vector<std::int16_t> pcm;
    pcm.reserve(blocks * 28);
    int s1 = 0, s2 = 0;
    for (std::size_t b = 0; b < blocks; ++b) {
        const unsigned char* block = data + b * 16;
        const int shift = block[0] & 15, filter = std::min((block[0] >> 4) & 7, 4);
        for (int i = 2; i < 16; ++i) {
            for (int nibble : {block[i] & 15, block[i] >> 4}) {
                int v = (nibble > 7 ? nibble - 16 : nibble) * 4096 >> shift;
                v += (s1 * filters[filter][0] + s2 * filters[filter][1] + 32) >> 6;
                v = std::clamp(v, -32768, 32767);
                pcm.push_back(static_cast<std::int16_t>(v));
                s2 = s1;
                s1 = v;
            }
        }
    }
    return pcm;
}

bool writeWav(const std::string& name, const std::vector<std::int16_t>& pcm) {
    std::ofstream out(name, std::ios::binary);
    const std::uint32_t bytes = static_cast<std::uint32_t>(pcm.size() * 2);
    const auto put32 = [&](std::uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); };
    const auto put16 = [&](std::uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); };
    out.write("RIFF", 4); put32(36 + bytes); out.write("WAVEfmt ", 8);
    put32(16); put16(1); put16(1); put32(SAMPLE_RATE); put32(SAMPLE_RATE * 2); put16(2); put16(16);
    out.write("data", 4); put32(bytes);
    out.write(reinterpret_cast<const char*>(pcm.data()), bytes);
    return static_cast<bool>(out);
}

}

int extractPlayStationSounds(const std::string& filename) {
    std::vector<ContainerEntry> entries;
    if (!readContainer(filename, entries)) return 0;
    std::ifstream input(filename, std::ios::binary);
    int written = 0;
    for (std::size_t i = 0; i + 1 < entries.size(); ++i) {
        const auto& header = entries[i];
        const auto& samples = entries[i + 1];
        if (header.type != 3 || samples.type != 4 || header.size < 4) continue;
        char magic[4] = {};
        input.seekg(static_cast<std::streamoff>(header.offset));
        input.read(magic, 4);
        if (!input || std::memcmp(magic, "Gian", 4) != 0) continue;

        std::vector<unsigned char> spu(samples.size);
        input.seekg(static_cast<std::streamoff>(samples.offset));
        input.read(reinterpret_cast<char*>(spu.data()), spu.size());
        if (!input) return written;
        // Split at end-flagged blocks; skip silent padding and the one-block terminators.
        static const unsigned char zero[16] = {};
        std::size_t start = SIZE_MAX;
        for (std::size_t o = 0; o + 16 <= spu.size(); o += 16) {
            if (start == SIZE_MAX) {
                if (std::memcmp(spu.data() + o, zero, 16) == 0) continue;
                start = o;
            }
            const bool end = (spu[o + 1] & 1) || o + 32 > spu.size();
            if (!end) continue;
            const std::size_t blocks = (o + 16 - start) / 16;
            if (blocks > 1) {
                const std::string name = filename + "." + std::to_string(written) + ".WAV";
                if (writeWav(name, decodeAdpcm(spu.data() + start, blocks))) {
                    std::cout << "[INFO] Saved PlayStation sound " << name << '\n';
                    ++written;
                }
            }
            start = SIZE_MAX;
        }
    }
    return written;
}

}
