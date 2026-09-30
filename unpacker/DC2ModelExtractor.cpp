#include "DC2ModelExtractor.h"

#include <array>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <QColor>
#include <QImage>
#include <QVector>

namespace {
constexpr std::uint32_t sector = 0x800;
struct Entry { std::uint32_t type, size, address, reserved; };
std::uint32_t align(std::uint32_t n) { return (n + sector - 1) & ~(sector - 1); }

// Texture pages are stored as 64x32 tiles (one 0x800 sector each), left to right then top
// to bottom: RESULT pages are 2 tiles across (128x256), character pages 4 across (256x256).
std::vector<unsigned char> rearrangeTextureTiles(const std::vector<unsigned char>& source,
                                                 std::size_t tilesAcross) {
    constexpr std::size_t tilesDown = 8;
    constexpr std::size_t tileWidth = 64;
    constexpr std::size_t tileHeight = 32;
    constexpr std::size_t tileSize = tileWidth * tileHeight;
    if (source.size() != tilesAcross * tilesDown * tileSize) return {};

    std::vector<unsigned char> result;
    result.reserve(source.size());
    for (std::size_t tileY = 0; tileY < tilesDown; ++tileY) {
        for (std::size_t row = 0; row < tileHeight; ++row) {
            for (std::size_t tileX = 0; tileX < tilesAcross; ++tileX) {
                const std::size_t offset = (tileY * tilesAcross + tileX) * tileSize + row * tileWidth;
                result.insert(result.end(), source.begin() + offset, source.begin() + offset + tileWidth);
            }
        }
    }
    return result;
}

bool saveResultTexture(const std::vector<unsigned char>& indexed,
                       const std::vector<unsigned char>& palette,
                       const std::string& outputName) {
    const std::vector<unsigned char> pixels = rearrangeTextureTiles(indexed, 2);
    if (pixels.empty() || palette.size() != 0x200) return false;

    QVector<QRgb> colors;
    colors.reserve(256);
    for (std::size_t i = 0; i < palette.size(); i += 2) {
        const std::uint16_t value = static_cast<std::uint16_t>(palette[i]) |
                                    (static_cast<std::uint16_t>(palette[i + 1]) << 8);
        const int red = (value & 0x1f) * 255 / 31;
        const int green = ((value >> 5) & 0x1f) * 255 / 31;
        const int blue = ((value >> 10) & 0x1f) * 255 / 31;
        colors.push_back(QColor(red, green, blue).rgb());
    }

    QImage image(128, 256, QImage::Format_Indexed8);
    image.setColorTable(colors);
    for (int y = 0; y < image.height(); ++y)
        std::memcpy(image.scanLine(y), pixels.data() + y * image.width(), image.width());
    return image.save(QString::fromStdString(outputName), "PNG");
}

bool writeResultMaterials(const std::string& modelName, unsigned textureCount) {
    if (textureCount == 0) return false;
    const std::string materialName = modelName + ".mtl";
    std::ofstream out(materialName, std::ios::trunc);
    if (!out) return false;
    const std::string modelFileName = std::filesystem::path(modelName).filename().string();
    const std::size_t modelSuffix = modelFileName.find(".model.");
    const std::string texturePrefix = modelSuffix == std::string::npos
                                        ? modelFileName
                                        : modelFileName.substr(0, modelSuffix);
    for (unsigned i = 0; i < textureCount; ++i) {
        out << "newmtl result_texture_" << i << '\n'
            << "Ka 1.000000 1.000000 1.000000\n"
            << "Kd 1.000000 1.000000 1.000000\n"
            << "Ks 0.000000 0.000000 0.000000\n"
            << "d 1.000000\n"
            << "illum 1\n"
            << "map_Kd " << texturePrefix
            << ".texture." << i << ".tiled.png\n\n";
    }
    return static_cast<bool>(out);
}

// Character texture page (E*.DAT / WEP_*.DAT): a type-1 entry of 0x10000 bytes (256x256,
// 8 bits per pixel) followed by its 256-colour type-2 CLUT. Colour 0x0000 is transparent.
bool saveModelTexture(const std::vector<unsigned char>& indexed,
                      const std::vector<unsigned char>& palette,
                      const std::string& outputName) {
    const std::vector<unsigned char> pixels = rearrangeTextureTiles(indexed, 4);
    if (pixels.empty() || palette.size() != 0x200) return false;

    QVector<QRgb> colors;
    colors.reserve(256);
    for (std::size_t i = 0; i < palette.size(); i += 2) {
        const std::uint16_t value = static_cast<std::uint16_t>(palette[i]) |
                                    (static_cast<std::uint16_t>(palette[i + 1]) << 8);
        const int red = (value & 0x1f) * 255 / 31;
        const int green = ((value >> 5) & 0x1f) * 255 / 31;
        const int blue = ((value >> 10) & 0x1f) * 255 / 31;
        colors.push_back(value == 0 ? qRgba(0, 0, 0, 0) : QColor(red, green, blue).rgb());
    }

    QImage image(256, 256, QImage::Format_Indexed8);
    image.setColorTable(colors);
    for (int y = 0; y < image.height(); ++y)
        std::memcpy(image.scanLine(y), pixels.data() + y * image.width(), image.width());
    return image.save(QString::fromStdString(outputName), "PNG");
}

bool writeModelMaterial(const std::string& objName, const std::string& textureName) {
    std::ofstream out(objName + ".mtl", std::ios::trunc);
    if (!out) return false;
    out << "newmtl model_texture\n"
        << "Ka 1.000000 1.000000 1.000000\n"
        << "Kd 1.000000 1.000000 1.000000\n"
        << "Ks 0.000000 0.000000 0.000000\n"
        << "d 1.000000\n"
        << "illum 1\n"
        << "map_Kd " << std::filesystem::path(textureName).filename().string() << '\n';
    return static_cast<bool>(out);
}

bool decompress(const std::vector<unsigned char>& in, std::vector<unsigned char>& out) {
    std::size_t src = 0;
    while (src < in.size()) {
        unsigned flags = in[src++] | 0x100;
        for (int bit = 0; bit < 8 && src < in.size(); ++bit, flags >>= 1) {
            if (flags & 1) { out.push_back(in[src++]); continue; }
            if (src + 1 >= in.size()) return false;
            unsigned lo = in[src++], hi = in[src++];
            unsigned distance = lo | ((hi & 0x0f) << 8), length = (hi >> 4) + 2;
            if (distance == 0 || distance > out.size()) return false;
            for (unsigned i = 0; i < length; ++i) out.push_back(out[out.size() - distance]);
        }
    }
    return true;
}

std::uint16_t u16(const std::vector<unsigned char>& b, std::size_t o) {
    return b[o] | (static_cast<std::uint16_t>(b[o + 1]) << 8);
}
std::uint32_t u32(const std::vector<unsigned char>& b, std::size_t o) {
    return u16(b, o) | (static_cast<std::uint32_t>(u16(b, o + 2)) << 16);
}
// Character models: the type-5 block of E*.DAT (enemies) and WEP_*.DAT (playable
// characters), decompressed at its load address `base` (all pointers are absolute).
//   0x00 u32 vertex, normal, triangle and quad table pointers
//   0x10 u16 triangle total, quad total, part count, pad; 0x18 u32 tpage | clut << 16
//   0x1C part records, 0x14 bytes each:
//          s16 x, y, z   joint offset from the parent part (the root's is its position)
//          u8  parent    (0xFF for the root), u8 flags
//          u32 triangle pointer, u32 quad pointer, s16 triangle count, s16 quad count
//   vertices (8 bytes): s16 x, y, z in the local space of their part, u16 part index;
//          0x2E2E marks unused filler entries.  Normals use the same layout.
//   triangles (12 bytes): u16 v0, v1, v2, then u8 u, v x3 stored rotated by one corner
//          (uv of v2, v0, v1).  quads (16 bytes): u16 v0..v3 in PlayStation order
//          (top-left, top-right, bottom-left, bottom-right), then u8 u, v x4.
// A vertex's position is its part's bind-pose joint (the sum of the offsets up the parent
// chain) plus its local coordinates; in this bind pose the feet of E10/E40 sit exactly on
// y = 0.  The PlayStation space is y-down, so the OBJ is rotated 180 degrees about X.
// Animations (after the quad table in E files; at +0x3800 behind 0xCD padding in WEP files)
// are 0x88-byte frames: u16 time, u16 duration, s16 root x, y, z, three shorts of
// accumulated root motion, then one s16 x, y, z rotation (4096 = 360 degrees) per part.
bool exportCharacterObj(const std::vector<unsigned char>& b, std::uint32_t base,
                        const std::string& name, const std::string& textureName) {
    if (b.size() < 0x1C) return false;
    const std::uint32_t va = u32(b, 0), na = u32(b, 4), ta = u32(b, 8), qa = u32(b, 12);
    if (va < base || na < base || ta < base || qa < base) return false;
    const std::size_t v = va - base, n = na - base;
    const std::size_t partCount = u16(b, 0x14);
    if (partCount == 0 || v != 0x1C + partCount * 0x14 || n <= v || (n - v) % 8 ||
        n + (n - v) > b.size()) return false;
    const std::size_t vertices = (n - v) / 8;

    struct Part { std::array<std::int32_t, 3> joint; std::size_t tris, quads, triCount, quadCount; };
    std::vector<Part> parts(partCount);
    for (std::size_t k = 0; k < partCount; ++k) {
        const std::size_t r = 0x1C + k * 0x14;
        const std::array<std::int32_t, 3> offset = {static_cast<std::int16_t>(u16(b, r)),
                                                    static_cast<std::int16_t>(u16(b, r + 2)),
                                                    static_cast<std::int16_t>(u16(b, r + 4))};
        const unsigned parent = b[r + 6];
        Part& part = parts[k];
        part.joint = offset;
        if (parent != 0xFF) {
            if (parent >= k) return false;           // parents always precede their children
            for (int i = 0; i < 3; ++i) part.joint[i] += parts[parent].joint[i];
        }
        const std::uint32_t tp = u32(b, r + 8), qp = u32(b, r + 12);
        part.triCount = u16(b, r + 16);
        part.quadCount = u16(b, r + 18);
        if (tp < base || qp < base || tp - base + part.triCount * 12 > b.size() ||
            qp - base + part.quadCount * 16 > b.size()) return false;
        part.tris = tp - base;
        part.quads = qp - base;
    }

    std::ofstream out(name, std::ios::trunc);
    if (!out) return false;
    constexpr double scale = 0.001;
    out << "# Dino Crisis 2 character model, bind pose\n";
    if (!textureName.empty())
        out << "mtllib " << std::filesystem::path(name + ".mtl").filename().string() << '\n';
    out << "o " << std::filesystem::path(name).stem().string() << '\n';
    for (std::size_t i = 0; i < vertices; ++i) {
        const std::size_t o = v + i * 8;
        std::int32_t x = static_cast<std::int16_t>(u16(b, o));
        std::int32_t y = static_cast<std::int16_t>(u16(b, o + 2));
        std::int32_t z = static_cast<std::int16_t>(u16(b, o + 4));
        const std::uint16_t group = u16(b, o + 6);
        if (group < partCount) {
            x += parts[group].joint[0];
            y += parts[group].joint[1];
            z += parts[group].joint[2];
        }
        out << "v " << x * scale << ' ' << -y * scale << ' ' << -z * scale << '\n';
    }
    for (std::size_t i = 0; i < vertices; ++i) {
        const std::size_t o = n + i * 8;
        out << "vn " << static_cast<std::int16_t>(u16(b, o)) / 4096.0 << ' '
            << -static_cast<std::int16_t>(u16(b, o + 2)) / 4096.0 << ' '
            << -static_cast<std::int16_t>(u16(b, o + 4)) / 4096.0 << '\n';
    }

    std::size_t uvIndex = 1;
    const auto face = [&](std::size_t offset, std::size_t count) {
        std::array<std::size_t, 4> indices{};
        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t source = count == 4 && i >= 2 ? 5 - i : i;   // 0,1,3,2
            indices[i] = u16(b, offset + source * 2);
            if (indices[i] >= vertices) return false;
            const std::size_t uvSource = count == 3 ? (i + 1) % 3 : source;
            const std::size_t uv = offset + count * 2 + uvSource * 2;
            out << "vt " << b[uv] / 256.0 << ' ' << 1.0 - b[uv + 1] / 256.0 << '\n';
        }
        out << "f";
        for (std::size_t i = 0; i < count; ++i)
            out << ' ' << indices[i] + 1 << '/' << uvIndex + i << '/' << indices[i] + 1;
        out << '\n';
        uvIndex += count;
        return true;
    };
    for (std::size_t k = 0; k < partCount; ++k) {
        const Part& part = parts[k];
        if (part.triCount == 0 && part.quadCount == 0) continue;
        out << "g part_" << k << '\n';
        if (!textureName.empty()) out << "usemtl model_texture\n";
        for (std::size_t i = 0; i < part.triCount; ++i) if (!face(part.tris + i * 12, 3)) return false;
        for (std::size_t i = 0; i < part.quadCount; ++i) if (!face(part.quads + i * 16, 4)) return false;
    }
    if (!out) return false;
    return textureName.empty() || writeModelMaterial(name, textureName);
}

// RESULT.DAT uses the same static vertex/normal/primitive layout as the
// confirmed DOOR meshes.  Unlike E-models, its fourth vertex word is not a
// joint index, so applying the experimental skeletal transforms corrupts it.
bool exportStaticTexturedObj(const std::vector<unsigned char>& b, std::uint32_t base,
                             const std::string& name) {
    if (b.size() < 20) return false;
    const std::uint32_t va = u32(b, 0), na = u32(b, 4), ta = u32(b, 8), qa = u32(b, 12);
    if (va < base || na < base || ta < base || qa < base) return false;
    const std::size_t v = va - base, n = na - base, t = ta - base, q = qa - base;
    const std::size_t triangles = u16(b, 16), quads = u16(b, 18);
    if (v != 0x30 || n < v || t < n || q < t || (n - v) % 8 || (t - n) % 8 ||
        t + triangles * 12 > b.size() || q + quads * 16 > b.size()) return false;
    const std::size_t vertices = (n - v) / 8;
    if (vertices == 0) return false;

    std::ofstream out(name, std::ios::trunc);
    if (!out) return false;
    out << "# Dino Crisis 2 RESULT static mesh\n"
        << "# Texture-page/material assignment has not yet been decoded.\n"
        << "mtllib " << std::filesystem::path(name + ".mtl").filename().string() << '\n'
        << "usemtl result_texture_0\n"
        << "o result_mesh_0\n";
    for (std::size_t i = 0; i < vertices; ++i) {
        const std::size_t o = v + i * 8;
        out << "v " << static_cast<std::int16_t>(u16(b, o)) * 0.001f << ' '
            << static_cast<std::int16_t>(u16(b, o + 2)) * 0.001f << ' '
            << static_cast<std::int16_t>(u16(b, o + 4)) * 0.001f << '\n';
    }
    for (std::size_t i = 0; i < vertices; ++i) {
        const std::size_t o = n + i * 8;
        out << "vn " << static_cast<std::int16_t>(u16(b, o)) / 4096.0f << ' '
            << static_cast<std::int16_t>(u16(b, o + 2)) / 4096.0f << ' '
            << static_cast<std::int16_t>(u16(b, o + 4)) / 4096.0f << '\n';
    }

    // One UV is emitted per face corner, preserving the original mapping.
    std::size_t uvIndex = 1;
    const auto writeFace = [&](std::size_t offset, std::size_t count) {
        std::array<std::size_t, 4> indices{};
        std::array<std::size_t, 4> uvs{};
        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t source = count == 4 && i >= 2 ? 5 - i : i;
            indices[i] = u16(b, offset + source * 2);
            if (indices[i] >= vertices) return false;
            const std::size_t uvSource = count == 3 ? (i + 1) % 3 : source;
            const std::size_t uv = offset + count * 2 + uvSource * 2;
            out << "vt " << b[uv] / 128.0f << ' ' << 1.0f - b[uv + 1] / 256.0f << '\n';
            uvs[i] = uvIndex++;
        }
        out << "f";
        for (std::size_t i = 0; i < count; ++i)
            out << ' ' << indices[i] + 1 << '/' << uvs[i] << '/' << indices[i] + 1;
        out << '\n';
        return true;
    };
    for (std::size_t i = 0; i < triangles; ++i) if (!writeFace(t + i * 12, 3)) return false;
    for (std::size_t i = 0; i < quads; ++i) if (!writeFace(q + i * 16, 4)) return false;
    return static_cast<bool>(out);
}
}

int DC2ModelExtractor::extract(const std::string& filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input) return 1;
    const bool isResult = std::filesystem::path(filename).filename() == "RESULT.DAT";
    std::uint32_t offset = sector;
    unsigned modelNumber = 0;
    unsigned resultTextureNumber = 0;
    std::vector<unsigned char> pendingResultTexture;
    std::vector<unsigned char> pendingModelTexture;
    unsigned modelTextureNumber = 0;
    std::string modelTexture;
    for (std::uint32_t entryOffset = 0; entryOffset < sector; entryOffset += 0x20) {
        Entry entry{};
        input.seekg(entryOffset);
        input.read(reinterpret_cast<char*>(&entry), sizeof(entry));
        if (!input) return 1;
        if (std::memcmp(&entry.type, "dummy header    ", 16) == 0) break;
        if (isResult && entry.type == 6) {
            std::vector<unsigned char> packed(entry.size);
            input.seekg(offset);
            input.read(reinterpret_cast<char*>(packed.data()), packed.size());
            std::vector<unsigned char> unpacked;
            if (input && decompress(packed, unpacked) && unpacked.size() == 0x8000) {
                pendingResultTexture = std::move(unpacked);
            } else {
                pendingResultTexture.clear();
            }
        } else if (isResult && entry.type == 2 && !pendingResultTexture.empty()) {
            std::vector<unsigned char> palette(entry.size);
            input.seekg(offset);
            input.read(reinterpret_cast<char*>(palette.data()), palette.size());
            const std::string textureName = filename + ".texture." + std::to_string(resultTextureNumber) + ".tiled.png";
            if (input && saveResultTexture(pendingResultTexture, palette, textureName)) {
                std::cout << "[INFO] Saved RESULT texture page: " << textureName << '\n';
                ++resultTextureNumber;
            }
            pendingResultTexture.clear();
        } else if (!isResult && entry.type == 1) {
            // Character texture page; saved once its CLUT (the next entry) is read.
            pendingModelTexture.assign(entry.size == 0x10000 ? entry.size : 0, 0);
            input.seekg(offset);
            input.read(reinterpret_cast<char*>(pendingModelTexture.data()), pendingModelTexture.size());
            if (!input) pendingModelTexture.clear();
        } else if (!isResult && entry.type == 2 && !pendingModelTexture.empty()) {
            std::vector<unsigned char> palette(entry.size);
            input.seekg(offset);
            input.read(reinterpret_cast<char*>(palette.data()), palette.size());
            const std::string textureName = filename + ".texture." + std::to_string(modelTextureNumber) + ".png";
            if (input && saveModelTexture(pendingModelTexture, palette, textureName)) {
                std::cout << "[INFO] Saved model texture page: " << textureName << '\n';
                if (modelTextureNumber++ == 0) modelTexture = textureName;   // the page models use
            }
            pendingModelTexture.clear();
        } else if (entry.type == 5) {
            std::vector<unsigned char> packed(entry.size);
            input.seekg(offset);
            input.read(reinterpret_cast<char*>(packed.data()), packed.size());
            if (!input) return 1;
            const std::string stem = filename + ".model." + std::to_string(modelNumber++);
            std::vector<unsigned char> unpacked;
            if (decompress(packed, unpacked)) {
                std::cout << "[INFO] Extracted DC2 model " << stem << " (" << packed.size() << " -> " << unpacked.size() << " bytes, load 0x" << std::hex << entry.address << std::dec << ")\n";
                if (isResult) {
                    if (exportStaticTexturedObj(unpacked, entry.address, stem + ".obj")) {
                        std::cout << "[INFO] Saved RESULT static mesh OBJ: " << stem << ".obj\n";
                    } else {
                        std::cout << "[WARNING] RESULT type-5 block did not match the confirmed static mesh layout.\n";
                    }
                    offset += align(entry.size);
                    continue;
                }
                std::ofstream(stem + ".compressed", std::ios::binary).write(reinterpret_cast<const char*>(packed.data()), packed.size());
                std::ofstream(stem + ".decompressed", std::ios::binary).write(reinterpret_cast<const char*>(unpacked.data()), unpacked.size());
                if (exportCharacterObj(unpacked, entry.address, stem + ".obj", modelTexture)) {
                    std::cout << "[INFO] Saved character model OBJ (bind pose): " << stem << ".obj\n";
                } else {
                    std::cout << "[WARNING] Type-5 block is not a character model: " << stem << '\n';
                }
            } else {
                std::cout << "[WARNING] Extracted type-5 block but DC2 LZSS decoding failed: " << stem << '\n';
            }
        }
        offset += align(entry.size);
    }
    if (isResult && writeResultMaterials(filename + ".model.0.obj", resultTextureNumber)) {
        std::cout << "[INFO] Saved RESULT texture material library.\n";
    }
    return 0;
}
