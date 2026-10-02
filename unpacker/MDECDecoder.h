#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dc2 {

// True when `data` starts a PlayStation MDEC "BS" version 2 frame: u16 code count, u16
// 0x3800, u16 quantisation scale, u16 version.
bool isBsFrame(const unsigned char* data, std::size_t size);

// Decodes a BS v2 frame (the PlayStation version's room backgrounds) to 24-bit RGB.
// The bitstream is 16-bit little-endian words read MSB first: per 16x16 macroblock (in
// columns, top to bottom) the Cr, Cb and four Y 8x8 blocks, each a 10-bit DC value then
// MPEG-1 run/level codes up to an end-of-block code. Returns false on a malformed stream.
bool decodeBsFrame(const unsigned char* data, std::size_t size, int width, int height,
                   std::vector<std::uint8_t>& rgb);

}
