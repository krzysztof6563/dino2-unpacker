#include "MDECDecoder.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <utility>

namespace dc2 {
namespace {

// MPEG-1 DCT coefficient codes (ISO 11172-2 table B.14, non-first coefficients) as
// "code run level"; a sign bit follows each code. "10" ends a block, "000001" escapes.
const char* const VLC_CODES[] = {
    "11 0 1", "011 1 1", "0100 0 2", "0101 2 1", "00101 0 3", "00111 3 1", "00110 4 1",
    "000110 1 2", "000111 5 1", "000101 6 1", "000100 7 1", "0000110 0 4", "0000100 2 2",
    "0000111 8 1", "0000101 9 1", "00100110 0 5", "00100001 0 6", "00100101 1 3", "00100100 3 2",
    "00100111 10 1", "00100011 11 1", "00100010 12 1", "00100000 13 1", "0000001010 0 7",
    "0000001100 1 4", "0000001011 2 3", "0000001111 4 2", "0000001001 5 2", "0000001110 14 1",
    "0000001101 15 1", "0000001000 16 1", "000000011101 0 8", "000000011000 0 9",
    "000000010011 0 10", "000000010000 0 11", "000000011011 1 5", "000000010100 2 4",
    "000000011100 3 3", "000000010010 4 3", "000000011110 6 2", "000000010101 7 2",
    "000000010001 8 2", "000000011111 17 1", "000000011010 18 1", "000000011001 19 1",
    "000000010111 20 1", "000000010110 21 1", "0000000011010 0 12", "0000000011001 0 13",
    "0000000011000 0 14", "0000000010111 0 15", "0000000010110 1 6", "0000000010101 1 7",
    "0000000010100 2 5", "0000000010011 3 4", "0000000010010 5 3", "0000000010001 9 2",
    "0000000010000 10 2", "0000000011111 22 1", "0000000011110 23 1", "0000000011101 24 1",
    "0000000011100 25 1", "0000000011011 26 1", "00000000011111 0 16", "00000000011110 0 17",
    "00000000011101 0 18", "00000000011100 0 19", "00000000011011 0 20", "00000000011010 0 21",
    "00000000011001 0 22", "00000000011000 0 23", "00000000010111 0 24", "00000000010110 0 25",
    "00000000010101 0 26", "00000000010100 0 27", "00000000010011 0 28", "00000000010010 0 29",
    "00000000010001 0 30", "00000000010000 0 31", "000000000011000 0 32", "000000000010111 0 33",
    "000000000010110 0 34", "000000000010101 0 35", "000000000010100 0 36",
    "000000000010011 0 37", "000000000010010 0 38", "000000000010001 0 39",
    "000000000010000 0 40", "000000000011111 1 8", "000000000011110 1 9",
    "000000000011101 1 10", "000000000011100 1 11", "000000000011011 1 12",
    "000000000011010 1 13", "000000000011001 1 14", "0000000000010011 1 15",
    "0000000000010010 1 16", "0000000000010001 1 17", "0000000000010000 1 18",
    "0000000000010100 6 3", "0000000000011010 11 2", "0000000000011001 12 2",
    "0000000000011000 13 2", "0000000000010111 14 2", "0000000000010110 15 2",
    "0000000000010101 16 2", "0000000000011111 27 1", "0000000000011110 28 1",
    "0000000000011101 29 1", "0000000000011100 30 1", "0000000000011011 31 1",
};

// The MDEC's quantisation table (the MPEG-1 default intra matrix with DC 2), indexed in
// zigzag order as the PlayStation uploads it.
const int QUANT[64] = {
    2, 16, 19, 22, 26, 27, 29, 34, 16, 16, 22, 24, 27, 29, 34, 37, 19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40, 22, 26, 27, 29, 32, 35, 40, 48, 26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69, 27, 29, 35, 38, 46, 56, 69, 83};
const int ZIGZAG[64] = {
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20,
    13, 6, 7, 14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59,
    52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};

using CodeKey = std::pair<int, std::uint32_t>;          // (length, bits)

const std::map<CodeKey, std::pair<int, int>>& codeTable() {
    static const auto table = [] {
        std::map<CodeKey, std::pair<int, int>> t;
        for (const char* entry : VLC_CODES) {
            std::uint32_t bits = 0;
            int length = 0;
            const char* p = entry;
            for (; *p != ' '; ++p, ++length) bits = bits << 1 | (*p - '0');
            int run = 0, level = 0;
            for (++p; *p != ' '; ++p) run = run * 10 + (*p - '0');
            for (++p; *p; ++p) level = level * 10 + (*p - '0');
            t[{length, bits}] = {run, level};
        }
        return t;
    }();
    return table;
}

const std::array<std::array<double, 8>, 8>& cosines() {
    static const auto table = [] {
        std::array<std::array<double, 8>, 8> c{};
        for (int x = 0; x < 8; ++x)
            for (int u = 0; u < 8; ++u)
                c[x][u] = (u ? 1.0 : std::sqrt(0.5)) * std::cos((2 * x + 1) * u * 3.14159265358979323846 / 16) / 2;
        return c;
    }();
    return table;
}

class BitReader {
public:
    BitReader(const unsigned char* data, std::size_t size) : data(data), bitsAvailable(size / 2 * 16) {}
    bool read(int count, std::uint32_t& value) {
        value = 0;
        for (int i = 0; i < count; ++i) {
            if (position >= bitsAvailable) return false;
            const std::size_t word = position >> 4;
            const unsigned v = data[2 * word] | data[2 * word + 1] << 8;
            value = value << 1 | (v >> (15 - (position & 15)) & 1);
            ++position;
        }
        return true;
    }
private:
    const unsigned char* data;
    std::size_t bitsAvailable, position = 0;
};

int signExtend(std::uint32_t value, int bits) {
    return (value & (1u << (bits - 1))) ? int(value) - (1 << bits) : int(value);
}

bool decodeBlock(BitReader& bits, int qscale, std::array<double, 64>& pixels) {
    std::array<double, 64> coef{};
    std::uint32_t v;
    if (!bits.read(10, v)) return false;
    coef[0] = signExtend(v, 10) * QUANT[0];
    const auto& table = codeTable();
    for (int k = 0;;) {
        std::uint32_t code = 0;
        int length = 0, run = 0, level = 0;
        for (;;) {
            std::uint32_t bit;
            if (!bits.read(1, bit) || ++length > 17) return false;
            code = code << 1 | bit;
            if (length == 2 && code == 0b10) break;                      // end of block
            if (length == 6 && code == 0b000001) {                       // escape
                std::uint32_t r, l;
                if (!bits.read(6, r) || !bits.read(10, l)) return false;
                run = int(r);
                level = signExtend(l, 10);
                break;
            }
            const auto it = table.find({length, code});
            if (it != table.end()) {
                std::uint32_t sign;
                if (!bits.read(1, sign)) return false;
                run = it->second.first;
                level = sign ? -it->second.second : it->second.second;
                break;
            }
        }
        if (length == 2 && code == 0b10) break;
        k += run + 1;
        if (k > 63) return false;
        const int magnitude = (std::abs(level) * QUANT[k] * qscale + 4) / 8;
        coef[ZIGZAG[k]] = level < 0 ? -magnitude : magnitude;
    }
    // Separable inverse DCT.
    const auto& c = cosines();
    std::array<double, 64> rows{};
    for (int v = 0; v < 8; ++v)
        for (int x = 0; x < 8; ++x) {
            double s = 0;
            for (int u = 0; u < 8; ++u) s += c[x][u] * coef[v * 8 + u];
            rows[v * 8 + x] = s;
        }
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            double s = 0;
            for (int v = 0; v < 8; ++v) s += c[y][v] * rows[v * 8 + x];
            pixels[y * 8 + x] = s;
        }
    return true;
}

}

bool isBsFrame(const unsigned char* data, std::size_t size) {
    return size >= 8 && data[2] == 0x00 && data[3] == 0x38 && data[6] == 2 && data[7] == 0;
}

bool decodeBsFrame(const unsigned char* data, std::size_t size, int width, int height,
                   std::vector<std::uint8_t>& rgb) {
    if (!isBsFrame(data, size) || width % 16 || height % 16) return false;
    const int qscale = data[4] | data[5] << 8;
    BitReader bits(data + 8, size - 8);
    rgb.assign(std::size_t(width) * height * 3, 0);
    std::array<double, 64> cr, cb, y[4];
    const auto clamp = [](double v) { return std::uint8_t(std::clamp(int(std::lround(v + 128)), 0, 255)); };
    for (int mx = 0; mx < width / 16; ++mx) {
        for (int my = 0; my < height / 16; ++my) {
            if (!decodeBlock(bits, qscale, cr) || !decodeBlock(bits, qscale, cb)) return false;
            for (auto& block : y)
                if (!decodeBlock(bits, qscale, block)) return false;
            for (int py = 0; py < 16; ++py)
                for (int px = 0; px < 16; ++px) {
                    const double luma = y[(py / 8) * 2 + px / 8][(py % 8) * 8 + px % 8];
                    const int c = (py / 2) * 8 + px / 2;
                    std::uint8_t* out = &rgb[((std::size_t(my) * 16 + py) * width + mx * 16 + px) * 3];
                    out[0] = clamp(luma + 1.402 * cr[c]);
                    out[1] = clamp(luma - 0.3437 * cb[c] - 0.7143 * cr[c]);
                    out[2] = clamp(luma + 1.772 * cb[c]);
                }
        }
    }
    return true;
}

}
