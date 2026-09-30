#include "DC2ModelExtractor.h"

#include <array>
#include <cctype>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <string>
#include <utility>
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
// to bottom: RESULT pages are 2 tiles across (128x256), character pages 4 across (256x256),
// or 2 across and 4 down (128x128) for small characters such as the Compsognathus.
std::vector<unsigned char> rearrangeTextureTiles(const std::vector<unsigned char>& source,
                                                 std::size_t tilesAcross, std::size_t tilesDown = 8) {
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
// 8 bits per pixel) or 0x4000 bytes (128x128, the top-left corner of the page) followed by
// its 256-colour type-2 CLUT. Colour 0x0000 is transparent. The PNG is always the full
// 256x256 page, so model UVs map the same way whatever the stored size.
bool isModelTextureSize(std::size_t size) { return size == 0x10000 || size == 0x4000; }

bool saveModelTexture(const std::vector<unsigned char>& indexed,
                      const std::vector<unsigned char>& palette,
                      const std::string& outputName) {
    const bool full = indexed.size() == 0x10000;
    const int width = full ? 256 : 128;
    const std::vector<unsigned char> pixels = rearrangeTextureTiles(indexed, full ? 4 : 2, full ? 8 : 4);
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

    colors.push_back(qRgba(0, 0, 0, 0));   // index 256: the unused part of a small page
    QImage image(256, 256, QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x)
            line[x] = colors[x < width && y < width ? pixels[y * width + x] : 256];
    }
    return image.save(QString::fromStdString(outputName), "PNG");
}

// VRAM position of the page a model samples, from the tpage in its header (0x18), in the
// same form as the load address of a type-1 entry: y << 16 | x (x in 16-bit units).
std::uint32_t modelTexturePageAddress(const std::vector<unsigned char>& model) {
    if (model.size() < 0x1A) return 0;
    const unsigned tpage = model[0x18] | model[0x19] << 8;
    return (tpage >> 4 & 1) * 256 << 16 | (tpage & 15) * 64;
}

// Reads the texture page loaded at `address` (a type-1 entry and the type-2 CLUT after it)
// from another DAT file and saves it as a PNG.
bool saveModelTextureFrom(const std::filesystem::path& datName, std::uint32_t address,
                          const std::string& outputName) {
    std::ifstream input(datName, std::ios::binary);
    std::uint32_t offset = sector;
    Entry previous{};
    std::uint32_t previousOffset = 0;
    for (std::uint32_t entryOffset = 0; input && entryOffset < sector; entryOffset += 0x20) {
        Entry entry{};
        input.seekg(entryOffset);
        input.read(reinterpret_cast<char*>(&entry), sizeof(entry));
        if (!input || std::memcmp(&entry.type, "dummy header    ", 16) == 0) break;
        if (entry.type == 2 && previous.type == 1 && previous.address == address &&
            isModelTextureSize(previous.size) && entry.size == 0x200) {
            std::vector<unsigned char> indexed(previous.size), palette(entry.size);
            input.seekg(previousOffset);
            input.read(reinterpret_cast<char*>(indexed.data()), indexed.size());
            input.seekg(offset);
            input.read(reinterpret_cast<char*>(palette.data()), palette.size());
            return input && saveModelTexture(indexed, palette, outputName);
        }
        previous = entry;
        previousOffset = offset;
        offset += align(entry.size);
    }
    return false;
}

// Some models do not carry the texture page they use; the game has it in VRAM from another
// file. The default-outfit player models WEP_P000 (Regina) and WEP_P100 (Dylan) take theirs
// from WP00A.DAT / WP10A.DAT (WEP_Pabc -> WPacA). E00 (Velociraptor) and E90 (Oviraptor)
// take the shared enemy page from whichever room file is loaded (SC*.DAT, ST502.DAT), so
// there is no single right file and they are exported untextured.
std::filesystem::path companionTextureFile(const std::string& filename) {
    const std::filesystem::path path(filename);
    std::string name = path.filename().string();
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::toupper(c); });
    if (name.size() != 12 || name.compare(0, 5, "WEP_P") != 0 || name.compare(8, 4, ".DAT") != 0) return {};
    const std::filesystem::path companion = path.parent_path() / ("WP" + name.substr(5, 1) + name.substr(7, 1) + "A.DAT");
    return std::filesystem::exists(companion) ? companion : std::filesystem::path();
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
// The primitives wind clockwise seen from outside (95% of faces oppose their vertex
// normals; the rest are deliberate back faces such as the inside of the mouth), so both
// exporters reverse them to the counter-clockwise front faces OBJ and glTF expect.
struct CharacterModel {
    struct Part {
        std::array<std::int32_t, 3> offset;   // from the parent joint
        std::array<std::int32_t, 3> joint;    // bind-pose position
        int parent;                           // -1 for a root
        std::size_t tris, quads, triCount, quadCount;
    };
    std::vector<Part> parts;
    std::size_t vertexTable = 0, normalTable = 0, vertices = 0;

    // Bind-pose position of vertex i, still in PlayStation units and axes.
    std::array<std::int32_t, 3> position(const std::vector<unsigned char>& b, std::size_t i) const {
        const std::size_t o = vertexTable + i * 8;
        std::array<std::int32_t, 3> p = {static_cast<std::int16_t>(u16(b, o)),
                                         static_cast<std::int16_t>(u16(b, o + 2)),
                                         static_cast<std::int16_t>(u16(b, o + 4))};
        const std::uint16_t group = u16(b, o + 6);
        if (group < parts.size())
            for (int k = 0; k < 3; ++k) p[k] += parts[group].joint[k];
        return p;
    }
    std::size_t part(const std::vector<unsigned char>& b, std::size_t i) const {
        const std::uint16_t group = u16(b, vertexTable + i * 8 + 6);
        return group < parts.size() ? group : 0;
    }
    std::array<double, 3> normal(const std::vector<unsigned char>& b, std::size_t i) const {
        const std::size_t o = normalTable + i * 8;
        return {static_cast<std::int16_t>(u16(b, o)) / 4096.0,
                static_cast<std::int16_t>(u16(b, o + 2)) / 4096.0,
                static_cast<std::int16_t>(u16(b, o + 4)) / 4096.0};
    }
};

bool parseCharacterModel(const std::vector<unsigned char>& b, std::uint32_t base, CharacterModel& m) {
    if (b.size() < 0x1C) return false;
    const std::uint32_t va = u32(b, 0), na = u32(b, 4), ta = u32(b, 8), qa = u32(b, 12);
    if (va < base || na < base || ta < base || qa < base) return false;
    const std::size_t v = va - base, n = na - base;
    const std::size_t partCount = u16(b, 0x14);
    if (partCount == 0 || v != 0x1C + partCount * 0x14 || n <= v || (n - v) % 8 ||
        n + (n - v) > b.size()) return false;
    m.vertexTable = v;
    m.normalTable = n;
    m.vertices = (n - v) / 8;
    m.parts.assign(partCount, {});
    for (std::size_t k = 0; k < partCount; ++k) {
        const std::size_t r = 0x1C + k * 0x14;
        CharacterModel::Part& part = m.parts[k];
        part.offset = {static_cast<std::int16_t>(u16(b, r)),
                       static_cast<std::int16_t>(u16(b, r + 2)),
                       static_cast<std::int16_t>(u16(b, r + 4))};
        part.joint = part.offset;
        part.parent = b[r + 6] == 0xFF ? -1 : b[r + 6];
        if (part.parent >= 0) {
            if (static_cast<std::size_t>(part.parent) >= k) return false;   // parents come first
            for (int i = 0; i < 3; ++i) part.joint[i] += m.parts[part.parent].joint[i];
        }
        const std::uint32_t tp = u32(b, r + 8), qp = u32(b, r + 12);
        part.triCount = u16(b, r + 16);
        part.quadCount = u16(b, r + 18);
        if (tp < base || qp < base || tp - base + part.triCount * 12 > b.size() ||
            qp - base + part.quadCount * 16 > b.size()) return false;
        part.tris = tp - base;
        part.quads = qp - base;
    }
    // every primitive must reference a real vertex
    for (const auto& part : m.parts) {
        for (std::size_t i = 0; i < part.triCount * 3; ++i)
            if (u16(b, part.tris + (i / 3) * 12 + (i % 3) * 2) >= m.vertices) return false;
        for (std::size_t i = 0; i < part.quadCount * 4; ++i)
            if (u16(b, part.quads + (i / 4) * 16 + (i % 4) * 2) >= m.vertices) return false;
    }
    return true;
}

// Corners of one primitive as (vertex index, uv byte offset), counter-clockwise.  Triangles
// store their UVs rotated by one corner; quads are split along the 1-2 diagonal, as the
// PlayStation GPU draws them.
using Corner = std::pair<std::size_t, std::size_t>;
std::vector<std::array<Corner, 3>> primitiveTriangles(const std::vector<unsigned char>& b,
                                                      std::size_t offset, std::size_t count) {
    const auto corner = [&](std::size_t i) {
        const std::size_t uvSource = count == 3 ? (i + 1) % 3 : i;
        return Corner{u16(b, offset + i * 2), offset + count * 2 + uvSource * 2};
    };
    if (count == 3) return {{corner(0), corner(2), corner(1)}};
    return {{corner(0), corner(2), corner(1)}, {corner(1), corner(2), corner(3)}};
}

bool exportCharacterObj(const std::vector<unsigned char>& b, std::uint32_t base,
                        const std::string& name, const std::string& textureName) {
    CharacterModel m;
    if (!parseCharacterModel(b, base, m)) return false;

    std::ofstream out(name, std::ios::trunc);
    if (!out) return false;
    constexpr double scale = 0.001;
    out << "# Dino Crisis 2 character model, bind pose\n";
    if (!textureName.empty())
        out << "mtllib " << std::filesystem::path(name + ".mtl").filename().string() << '\n';
    out << "o " << std::filesystem::path(name).stem().string() << '\n';
    for (std::size_t i = 0; i < m.vertices; ++i) {
        const auto p = m.position(b, i);
        out << "v " << p[0] * scale << ' ' << -p[1] * scale << ' ' << -p[2] * scale << '\n';
    }
    for (std::size_t i = 0; i < m.vertices; ++i) {
        const auto nrm = m.normal(b, i);
        out << "vn " << nrm[0] << ' ' << -nrm[1] << ' ' << -nrm[2] << '\n';
    }

    std::size_t uvIndex = 1;
    const auto face = [&](std::size_t offset, std::size_t count) {
        // perimeter reversed to counter-clockwise: triangles 0,2,1; quads 0,2,3,1
        static constexpr std::size_t triOrder[] = {0, 2, 1}, quadOrder[] = {0, 2, 3, 1};
        const std::size_t* order = count == 3 ? triOrder : quadOrder;
        std::array<std::size_t, 4> indices{};
        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t source = order[i];
            indices[i] = u16(b, offset + source * 2);
            const std::size_t uvSource = count == 3 ? (source + 1) % 3 : source;
            const std::size_t uv = offset + count * 2 + uvSource * 2;
            out << "vt " << b[uv] / 256.0 << ' ' << 1.0 - b[uv + 1] / 256.0 << '\n';
        }
        out << "f";
        for (std::size_t i = 0; i < count; ++i)
            out << ' ' << indices[i] + 1 << '/' << uvIndex + i << '/' << indices[i] + 1;
        out << '\n';
        uvIndex += count;
    };
    for (std::size_t k = 0; k < m.parts.size(); ++k) {
        const auto& part = m.parts[k];
        if (part.triCount == 0 && part.quadCount == 0) continue;
        out << "g part_" << k << '\n';
        if (!textureName.empty()) out << "usemtl model_texture\n";
        for (std::size_t i = 0; i < part.triCount; ++i) face(part.tris + i * 12, 3);
        for (std::size_t i = 0; i < part.quadCount; ++i) face(part.quads + i * 16, 4);
    }
    if (!out) return false;
    return textureName.empty() || writeModelMaterial(name, textureName);
}

std::string base64(const std::vector<unsigned char>& data) {
    static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve((data.size() + 2) / 3 * 4);
    for (std::size_t i = 0; i < data.size(); i += 3) {
        const std::uint32_t n = static_cast<std::uint32_t>(data[i]) << 16 |
                                (i + 1 < data.size() ? static_cast<std::uint32_t>(data[i + 1]) << 8 : 0) |
                                (i + 2 < data.size() ? data[i + 2] : 0);
        out += table[n >> 18 & 63];
        out += table[n >> 12 & 63];
        out += i + 1 < data.size() ? table[n >> 6 & 63] : '=';
        out += i + 2 < data.size() ? table[n & 63] : '=';
    }
    return out;
}

// Rigged glTF 2.0: one joint node per part (hierarchy and bind-pose offsets from the part
// table) and every vertex bound with weight 1 to its own part, as the game skins them.
// Same units and axes as the OBJ (metres, +Y up).  Vertices are emitted per distinct
// (vertex, UV) corner because the game stores UVs per corner.  The buffer and the texture
// page PNG are embedded in the .gltf, so the file stands alone.
bool exportCharacterGltf(const std::vector<unsigned char>& b, std::uint32_t base,
                         const std::string& name, const std::string& textureName) {
    CharacterModel m;
    if (!parseCharacterModel(b, base, m)) return false;
    constexpr float scale = 0.001f;
    const auto toGltf = [&](const std::array<std::int32_t, 3>& p) {
        return std::array<float, 3>{p[0] * scale, -p[1] * scale, -p[2] * scale};
    };

    std::vector<float> positions, normals, uvs, weights;
    std::vector<std::uint16_t> joints;
    std::vector<std::uint32_t> indices;
    std::vector<std::pair<std::size_t, std::uint16_t>> keys;   // (vertex, packed uv) per output vertex
    std::array<float, 3> lo{1e9f, 1e9f, 1e9f}, hi{-1e9f, -1e9f, -1e9f};
    const auto addCorner = [&](const Corner& c) {
        const std::uint16_t uv = u16(b, c.second);
        for (std::size_t k = 0; k < keys.size(); ++k)            // models have a few hundred vertices
            if (keys[k].first == c.first && keys[k].second == uv) return static_cast<std::uint32_t>(k);
        keys.emplace_back(c.first, uv);
        const auto p = toGltf(m.position(b, c.first));
        for (int k = 0; k < 3; ++k) {
            positions.push_back(p[k]);
            lo[k] = std::min(lo[k], p[k]);
            hi[k] = std::max(hi[k], p[k]);
        }
        const auto nrm = m.normal(b, c.first);
        const double len = std::sqrt(nrm[0] * nrm[0] + nrm[1] * nrm[1] + nrm[2] * nrm[2]);
        if (len > 0)
            normals.insert(normals.end(), {static_cast<float>(nrm[0] / len), static_cast<float>(-nrm[1] / len),
                                           static_cast<float>(-nrm[2] / len)});
        else
            normals.insert(normals.end(), {0.0f, 1.0f, 0.0f});
        uvs.insert(uvs.end(), {b[c.second] / 256.0f, b[c.second + 1] / 256.0f});
        joints.insert(joints.end(), {static_cast<std::uint16_t>(m.part(b, c.first)), 0, 0, 0});
        weights.insert(weights.end(), {1.0f, 0.0f, 0.0f, 0.0f});
        return static_cast<std::uint32_t>(keys.size() - 1);
    };
    for (const auto& part : m.parts) {
        for (std::size_t i = 0; i < part.triCount + part.quadCount; ++i) {
            const bool tri = i < part.triCount;
            const std::size_t offset = tri ? part.tris + i * 12 : part.quads + (i - part.triCount) * 16;
            for (const auto& t : primitiveTriangles(b, offset, tri ? 3 : 4))
                for (const auto& c : t) indices.push_back(addCorner(c));
        }
    }
    const std::size_t count = keys.size();
    if (count == 0) return false;
    std::vector<float> inverseBind;   // column-major: translation by minus each joint's bind position
    for (const auto& part : m.parts) {
        const auto j = toGltf(part.joint);
        inverseBind.insert(inverseBind.end(), {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -j[0], -j[1], -j[2], 1});
    }

    std::vector<unsigned char> buffer;   // one view per accessor, each 4-byte aligned
    struct View { std::size_t offset, length; int target; };
    std::vector<View> views;
    const auto addView = [&](const void* data, std::size_t length, int target) {
        views.push_back({buffer.size(), length, target});
        const auto* bytes = static_cast<const unsigned char*>(data);
        buffer.insert(buffer.end(), bytes, bytes + length);
        while (buffer.size() % 4) buffer.push_back(0);
    };
    addView(positions.data(), positions.size() * 4, 34962);   // ARRAY_BUFFER
    addView(normals.data(), normals.size() * 4, 34962);
    addView(uvs.data(), uvs.size() * 4, 34962);
    addView(joints.data(), joints.size() * 2, 34962);
    addView(weights.data(), weights.size() * 4, 34962);
    addView(indices.data(), indices.size() * 4, 34963);       // ELEMENT_ARRAY_BUFFER
    addView(inverseBind.data(), inverseBind.size() * 4, 0);

    std::ofstream out(name, std::ios::trunc);
    if (!out) return false;
    out.precision(9);
    const std::size_t partCount = m.parts.size();
    out << "{\n\"asset\":{\"version\":\"2.0\",\"generator\":\"dino2-unpacker\"},\n\"scene\":0,\n";
    out << "\"scenes\":[{\"nodes\":[";
    for (std::size_t k = 0; k < partCount; ++k)
        if (m.parts[k].parent < 0) out << k << ',';
    out << partCount << "]}],\n\"nodes\":[\n";
    for (std::size_t k = 0; k < partCount; ++k) {
        const auto t = toGltf(m.parts[k].offset);
        out << "{\"name\":\"part_" << k << "\",\"translation\":[" << t[0] << ',' << t[1] << ',' << t[2] << ']';
        std::string children;
        for (std::size_t c = 0; c < partCount; ++c)
            if (m.parts[c].parent == static_cast<int>(k))
                children += (children.empty() ? "" : ",") + std::to_string(c);
        if (!children.empty()) out << ",\"children\":[" << children << ']';
        out << "},\n";
    }
    out << "{\"name\":\"" << std::filesystem::path(name).stem().string() << "\",\"mesh\":0,\"skin\":0}\n],\n";
    out << "\"skins\":[{\"inverseBindMatrices\":6,\"joints\":[";
    for (std::size_t k = 0; k < partCount; ++k) out << (k ? "," : "") << k;
    out << "]}],\n";
    out << "\"meshes\":[{\"primitives\":[{\"attributes\":{\"POSITION\":0,\"NORMAL\":1,\"TEXCOORD_0\":2,"
        << "\"JOINTS_0\":3,\"WEIGHTS_0\":4},\"indices\":5,\"material\":0}]}],\n";
    out << "\"materials\":[{\"name\":\"model_texture\",\"pbrMetallicRoughness\":{";
    if (!textureName.empty()) out << "\"baseColorTexture\":{\"index\":0},";
    out << "\"metallicFactor\":0,\"roughnessFactor\":1}";
    if (!textureName.empty()) out << ",\"alphaMode\":\"MASK\",\"alphaCutoff\":0.5";
    out << "}],\n";
    if (!textureName.empty()) {
        std::ifstream png(textureName, std::ios::binary | std::ios::ate);
        std::vector<unsigned char> pngData(png ? static_cast<std::size_t>(png.tellg()) : 0);
        png.seekg(0);
        png.read(reinterpret_cast<char*>(pngData.data()), pngData.size());
        out << "\"images\":[{\"uri\":\"";
        if (png && !pngData.empty())
            out << "data:image/png;base64," << base64(pngData);
        else
            out << std::filesystem::path(textureName).filename().string();
        out << "\"}],\n"
            << "\"samplers\":[{\"magFilter\":9728,\"minFilter\":9728,\"wrapS\":33071,\"wrapT\":33071}],\n"
            << "\"textures\":[{\"source\":0,\"sampler\":0}],\n";
    }
    out << "\"accessors\":[\n"
        << "{\"bufferView\":0,\"componentType\":5126,\"count\":" << count << ",\"type\":\"VEC3\",\"min\":["
        << lo[0] << ',' << lo[1] << ',' << lo[2] << "],\"max\":[" << hi[0] << ',' << hi[1] << ',' << hi[2] << "]},\n"
        << "{\"bufferView\":1,\"componentType\":5126,\"count\":" << count << ",\"type\":\"VEC3\"},\n"
        << "{\"bufferView\":2,\"componentType\":5126,\"count\":" << count << ",\"type\":\"VEC2\"},\n"
        << "{\"bufferView\":3,\"componentType\":5123,\"count\":" << count << ",\"type\":\"VEC4\"},\n"
        << "{\"bufferView\":4,\"componentType\":5126,\"count\":" << count << ",\"type\":\"VEC4\"},\n"
        << "{\"bufferView\":5,\"componentType\":5125,\"count\":" << indices.size() << ",\"type\":\"SCALAR\"},\n"
        << "{\"bufferView\":6,\"componentType\":5126,\"count\":" << partCount << ",\"type\":\"MAT4\"}\n],\n";
    out << "\"bufferViews\":[\n";
    for (std::size_t i = 0; i < views.size(); ++i) {
        out << "{\"buffer\":0,\"byteOffset\":" << views[i].offset << ",\"byteLength\":" << views[i].length;
        if (views[i].target) out << ",\"target\":" << views[i].target;
        out << '}' << (i + 1 < views.size() ? ",\n" : "\n");
    }
    out << "],\n\"buffers\":[{\"byteLength\":" << buffer.size()
        << ",\"uri\":\"data:application/octet-stream;base64," << base64(buffer) << "\"}]\n}\n";
    return static_cast<bool>(out);
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
    std::uint32_t pendingModelTextureAddress = 0;
    unsigned modelTextureNumber = 0;
    std::vector<std::pair<std::uint32_t, std::string>> modelTextures;   // (VRAM address, PNG)
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
            pendingModelTexture.assign(isModelTextureSize(entry.size) ? entry.size : 0, 0);
            pendingModelTextureAddress = entry.address;
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
                modelTextures.emplace_back(pendingModelTextureAddress, textureName);
                ++modelTextureNumber;
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
                // Use the page the model's tpage points at, from this file or its companion.
                const std::uint32_t pageAddress = modelTexturePageAddress(unpacked);
                std::string modelTexture;
                for (const auto& [address, name] : modelTextures)
                    if (address == pageAddress) modelTexture = name;
                CharacterModel model;
                if (modelTexture.empty() && parseCharacterModel(unpacked, entry.address, model)) {
                    const auto companion = companionTextureFile(filename);
                    const std::string textureName = filename + ".texture." + std::to_string(modelTextureNumber) + ".png";
                    if (!companion.empty() && saveModelTextureFrom(companion, pageAddress, textureName)) {
                        std::cout << "[INFO] Saved model texture page from " << companion.filename().string() << ": " << textureName << '\n';
                        modelTextures.emplace_back(pageAddress, textureName);
                        ++modelTextureNumber;
                        modelTexture = textureName;
                    } else {
                        std::cout << "[INFO] The texture page this model uses is not in this file; the game loads it from "
                                     "another file (see the README). Exporting untextured.\n";
                    }
                }
                if (exportCharacterObj(unpacked, entry.address, stem + ".obj", modelTexture)) {
                    std::cout << "[INFO] Saved character model OBJ (bind pose): " << stem << ".obj\n";
                    if (exportCharacterGltf(unpacked, entry.address, stem + ".gltf", modelTexture))
                        std::cout << "[INFO] Saved rigged character model glTF: " << stem << ".gltf\n";
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
