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

std::vector<unsigned char> rearrangeTextureTiles(const std::vector<unsigned char>& source) {
    constexpr std::size_t tilesAcross = 2;
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
    const std::vector<unsigned char> pixels = rearrangeTextureTiles(indexed);
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
bool exportExperimentalObj(const std::vector<unsigned char>& b, std::uint32_t base, const std::string& name,
                           bool applyJointTranslations = false,
                           const std::vector<std::array<std::int16_t, 3>>* rotations = nullptr,
                           bool useJointPivots = false, bool composeHierarchy = false) {
    if (b.size() < 20) return false;
    const std::uint32_t va = u32(b, 0), na = u32(b, 4), ta = u32(b, 8), qa = u32(b, 12);
    if (va < base || na < base || ta < base || qa < base) return false;
    const std::size_t v = va - base, n = na - base, t = ta - base, q = qa - base;
    const std::size_t triangles = u16(b, 16), quads = u16(b, 18);
    if (n < v || t < n || q < t || (n - v) % 8 || (t - n) % 8 ||
        t + triangles * 12 > b.size() || q + quads * 16 > b.size()) return false;
    const std::size_t vertices = (n - v) / 8;
    struct Joint { std::int16_t x, y, z; };
    std::vector<Joint> joints;
    if (applyJointTranslations) {
        // E-models keep 0x14-byte draw-group records between the header and
        // vertices.  Their first three shorts line up with the per-vertex
        // group ID stored in the fourth vertex short.
        // The table has a 16-byte trailer before the vertex data.  (For
        // E10: 19 records at 0x20 plus the trailer, then vertices at 0x1ac.)
        if (v < 0x30 || (v - 0x20 - 0x10) % 0x14) return false;
        const std::size_t groupCount = (v - 0x20 - 0x10) / 0x14;
        joints.reserve(groupCount);
        for (std::size_t i = 0; i < groupCount; ++i) {
            const std::size_t o = 0x20 + i * 0x14;
            joints.push_back({static_cast<std::int16_t>(u16(b, o)),
                              static_cast<std::int16_t>(u16(b, o + 2)),
                              static_cast<std::int16_t>(u16(b, o + 16))});
        }
    }
    struct Pose { std::array<double, 9> r; std::array<double, 3> p; };
    std::vector<Pose> poses;
    if (composeHierarchy && rotations && joints.size() == 19 && rotations->size() == 19) {
        // Derived from the groups connected by each primitive range.  The
        // coordinates in the group table are absolute bind-joint positions.
        constexpr int parent[] = {-1, 0, 1, 2, 3, 1, 1, 0, 7, 8, 9, 0, 11, 12, 13, 0, 15, 16, 17};
        const auto multiply = [](const std::array<double, 9>& a, const std::array<double, 9>& c) {
            std::array<double, 9> r{};
            for (int row = 0; row < 3; ++row) for (int col = 0; col < 3; ++col)
                for (int k = 0; k < 3; ++k) r[row * 3 + col] += a[row * 3 + k] * c[k * 3 + col];
            return r;
        };
        const auto transform = [](const std::array<double, 9>& r, const std::array<double, 3>& v) {
            return std::array<double, 3>{r[0] * v[0] + r[1] * v[1] + r[2] * v[2],
                                         r[3] * v[0] + r[4] * v[1] + r[5] * v[2],
                                         r[6] * v[0] + r[7] * v[1] + r[8] * v[2]};
        };
        constexpr double pi = 3.14159265358979323846;
        for (std::size_t i = 0; i < joints.size(); ++i) {
            const auto& a = (*rotations)[i];
            const double sx = std::sin(a[0] * 2.0 * pi / 4096.0), cx = std::cos(a[0] * 2.0 * pi / 4096.0);
            const double sy = std::sin(a[1] * 2.0 * pi / 4096.0), cy = std::cos(a[1] * 2.0 * pi / 4096.0);
            const double sz = std::sin(a[2] * 2.0 * pi / 4096.0), cz = std::cos(a[2] * 2.0 * pi / 4096.0);
            const std::array<double, 9> local = {cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx,
                                                  sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx,
                                                  -sy, cy * sx, cy * cx};
            const std::array<double, 3> bind = {static_cast<double>(joints[i].x), static_cast<double>(joints[i].y), static_cast<double>(joints[i].z)};
            if (parent[i] < 0) {
                poses.push_back({local, bind});
            } else {
                const auto& parentPose = poses[parent[i]];
                const auto& parentBind = joints[parent[i]];
                const std::array<double, 3> offset = {bind[0] - parentBind.x, bind[1] - parentBind.y, bind[2] - parentBind.z};
                const auto transformedOffset = transform(parentPose.r, offset);
                poses.push_back({multiply(parentPose.r, local), {parentPose.p[0] + transformedOffset[0], parentPose.p[1] + transformedOffset[1], parentPose.p[2] + transformedOffset[2]}});
            }
        }
    }
    std::ofstream out(name, std::ios::trunc);
    if (!out) return false;
    out << "# Experimental DC2 mesh: geometry only\n";
    for (std::size_t i = 0; i < vertices; ++i) {
        const std::size_t o = v + i * 8;
        std::int32_t x = static_cast<std::int16_t>(u16(b, o));
        std::int32_t y = static_cast<std::int16_t>(u16(b, o + 2));
        std::int32_t z = static_cast<std::int16_t>(u16(b, o + 4));
        const std::uint16_t group = u16(b, o + 6);
        if (!poses.empty() && group < poses.size()) {
            const auto& bind = joints[group];
            const auto& pose = poses[group];
            const double lx = x - bind.x, ly = y - bind.y, lz = z - bind.z;
            x = static_cast<std::int32_t>(std::lround(pose.p[0] + pose.r[0] * lx + pose.r[1] * ly + pose.r[2] * lz));
            y = static_cast<std::int32_t>(std::lround(pose.p[1] + pose.r[3] * lx + pose.r[4] * ly + pose.r[5] * lz));
            z = static_cast<std::int32_t>(std::lround(pose.p[2] + pose.r[6] * lx + pose.r[7] * ly + pose.r[8] * lz));
        } else if (applyJointTranslations && group < joints.size()) {
            if (useJointPivots) {
                // Vertices are in the bind-pose coordinate space.  A group
                // rotation must be around its bind joint, not world zero.
                x -= joints[group].x;
                y -= joints[group].y;
                z -= joints[group].z;
            }
            if (rotations && group < rotations->size()) {
                const auto& r = (*rotations)[group];
                const double scale = 2.0 * 3.14159265358979323846 / 4096.0;
                const double sx = std::sin(r[0] * scale), cx = std::cos(r[0] * scale);
                const double sy = std::sin(r[1] * scale), cy = std::cos(r[1] * scale);
                const double sz = std::sin(r[2] * scale), cz = std::cos(r[2] * scale);
                double rx = x, ry = y, rz = z;
                // PlayStation's standard Euler order: X, then Y, then Z.
                const double y1 = ry * cx - rz * sx, z1 = ry * sx + rz * cx;
                const double x2 = rx * cy + z1 * sy, z2 = -rx * sy + z1 * cy;
                x = static_cast<std::int32_t>(std::lround(x2 * cz - y1 * sz));
                y = static_cast<std::int32_t>(std::lround(x2 * sz + y1 * cz));
                z = static_cast<std::int32_t>(std::lround(z2));
            }
            x += joints[group].x;
            y += joints[group].y;
            z += joints[group].z;
        }
        out << "v " << x << ' ' << y << ' ' << z << '\n';
    }
    const auto face = [&](std::size_t o, std::size_t count) {
        out << "f";
        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t source = count == 4 && i >= 2 ? 5 - i : i;
            const std::size_t index = u16(b, o + source * 2);
            if (index >= vertices) return false;
            out << ' ' << index + 1;
        }
        out << '\n';
        return true;
    };
    for (std::size_t i = 0; i < triangles; ++i) if (!face(t + i * 12, 3)) return false;
    for (std::size_t i = 0; i < quads; ++i) if (!face(q + i * 16, 4)) return false;
    return true;
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

bool readClip3Frame0Rotations(const std::vector<unsigned char>& b, std::uint32_t base,
                              std::vector<std::array<std::int16_t, 3>>& rotations) {
    if (b.size() < 20) return false;
    const std::size_t quadData = u32(b, 12) - base;
    const std::size_t animationTable = quadData + u16(b, 18) * 16;
    constexpr std::size_t clip = 3;
    if (animationTable + (clip + 2) * 4 > b.size()) return false;
    const std::uint32_t startAddress = u32(b, animationTable + clip * 4);
    const std::uint32_t endAddress = u32(b, animationTable + (clip + 1) * 4);
    if (startAddress < base || endAddress < startAddress) return false;
    const std::size_t start = startAddress - base, end = endAddress - base;
    // These clips contain 0x88-byte frames.  Clip 3 has its first frame at
    // +0x20; its first 19 triples are the per-group Euler rotations.
    if (start + 0x20 + 19 * 6 > end || u32(b, start) == 0) return false;
    rotations.clear();
    for (std::size_t group = 0; group < 19; ++group) {
        const std::size_t o = start + 0x20 + group * 6;
        rotations.push_back({static_cast<std::int16_t>(u16(b, o)),
                             static_cast<std::int16_t>(u16(b, o + 2)),
                             static_cast<std::int16_t>(u16(b, o + 4))});
    }
    return true;
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
                if (exportExperimentalObj(unpacked, entry.address, stem + ".experimental.obj")) {
                    std::cout << "[INFO] Saved experimental geometry OBJ: " << stem << ".experimental.obj\n";
                }
                if (exportExperimentalObj(unpacked, entry.address, stem + ".experimental.joint-translation.obj", true)) {
                    std::cout << "[INFO] Saved joint-translation OBJ candidate: " << stem << ".experimental.joint-translation.obj\n";
                }
                std::vector<std::array<std::int16_t, 3>> rotations;
                if (readClip3Frame0Rotations(unpacked, entry.address, rotations) &&
                    exportExperimentalObj(unpacked, entry.address, stem + ".experimental.pose-clip3-frame0.obj", true, &rotations, true)) {
                    std::cout << "[INFO] Saved pose OBJ candidate (clip 3, frame 0): " << stem << ".experimental.pose-clip3-frame0.obj\n";
                }
                if (readClip3Frame0Rotations(unpacked, entry.address, rotations) &&
                    exportExperimentalObj(unpacked, entry.address, stem + ".experimental.pose-clip3-frame0-hierarchy.obj", true, &rotations, true, true)) {
                    std::cout << "[INFO] Saved hierarchy pose OBJ candidate (clip 3, frame 0): " << stem << ".experimental.pose-clip3-frame0-hierarchy.obj\n";
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
