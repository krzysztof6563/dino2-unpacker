#include "DoorUnpacker.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>

namespace {

constexpr std::uint32_t SECTOR_SIZE = 0x800;
constexpr std::uint32_t DIRECTORY_SIZE = 0x800;
constexpr std::uint32_t ENTRY_SIZE = 0x20;
constexpr char DUMMY_HEADER[] = "dummy header    ";
constexpr float OBJ_SCALE = 0.001f;

struct DirectoryEntry {
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t address;
    std::uint32_t reserved;
};

std::uint32_t alignToSector(std::uint32_t value) {
    return (value + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1);
}

std::string fileNameOnly(const std::string& path) {
    const std::size_t separator = path.find_last_of("/\\");
    return separator == std::string::npos ? path : path.substr(separator + 1);
}

bool writeObjMaterial(const std::string& objName, const std::string& textureName) {
    std::ofstream material(objName + ".mtl", std::ios::trunc);
    if (!material) {
        return false;
    }
    material << "# Dino Crisis 2 DOOR texture material\n"
             << "newmtl door_texture\n"
             << "Ka 1.000000 1.000000 1.000000\n"
             << "Kd 1.000000 1.000000 1.000000\n"
             << "Ks 0.000000 0.000000 0.000000\n"
             << "d 1.000000\n"
             << "illum 1\n"
             << "map_Kd " << fileNameOnly(textureName) << '\n';
    return static_cast<bool>(material);
}

std::vector<std::uint8_t> rearrangeDoorTiles(
    const std::vector<unsigned char>& source,
    std::size_t tileRowBytes,
    std::size_t tileRows
) {
    constexpr std::size_t tilesAcross = 2;
    constexpr std::size_t tilesDown = 8;
    const std::size_t tileSize = tileRowBytes * tileRows;
    if (source.size() != tilesAcross * tilesDown * tileSize) {
        return {};
    }
    std::vector<std::uint8_t> result;
    result.reserve(source.size());
    for (std::size_t tileY = 0; tileY < tilesDown; ++tileY) {
        for (std::size_t row = 0; row < tileRows; ++row) {
            for (std::size_t tileX = 0; tileX < tilesAcross; ++tileX) {
                const std::size_t offset = (tileY * tilesAcross + tileX) * tileSize + row * tileRowBytes;
                result.insert(result.end(), source.begin() + offset, source.begin() + offset + tileRowBytes);
            }
        }
    }
    return result;
}

bool readAt(std::ifstream& file, std::uint32_t offset, std::vector<unsigned char>& data) {
    file.clear();
    file.seekg(offset, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), data.size());
    return file.good() || file.gcount() == static_cast<std::streamsize>(data.size());
}

std::uint16_t readU16(const std::vector<unsigned char>& data, std::size_t offset) {
    return static_cast<std::uint16_t>(data[offset]) |
           (static_cast<std::uint16_t>(data[offset + 1]) << 8);
}

std::uint32_t readU32(const std::vector<unsigned char>& data, std::size_t offset) {
    return static_cast<std::uint32_t>(readU16(data, offset)) |
           (static_cast<std::uint32_t>(readU16(data, offset + 2)) << 16);
}

bool decompressDc2Lzss(const std::vector<unsigned char>& packed, std::vector<unsigned char>& unpacked) {
    std::size_t source = 0;
    while (source < packed.size()) {
        unsigned int flags = static_cast<unsigned int>(packed[source++]) | 0x100;
        for (int bit = 0; bit < 8 && source < packed.size(); ++bit, flags >>= 1) {
            if (flags & 1) {
                unpacked.push_back(packed[source++]);
                continue;
            }
            if (source + 1 >= packed.size()) {
                return false;
            }
            const unsigned int low = packed[source++];
            const unsigned int high = packed[source++];
            const unsigned int distance = low | ((high & 0x0f) << 8);
            const unsigned int length = (high >> 4) + 2;
            if (distance == 0 || distance > unpacked.size()) {
                return false;
            }
            for (unsigned int i = 0; i < length; ++i) {
                unpacked.push_back(unpacked[unpacked.size() - distance]);
            }
        }
    }
    return true;
}

bool exportDc2MeshesToObj(
    const std::vector<unsigned char>& data,
    std::uint32_t loadAddress,
    int textureWidth,
    int textureHeight,
    const std::string& outputName,
    const std::string& doorFileName,
    bool rotateTriangleUvs
) {
    std::ofstream output;

    std::size_t headerOffset = 0;
    std::size_t vertexBase = 1;
    std::size_t uvBase = 1;
    std::size_t normalBase = 1;
    std::size_t objectNumber = 0;
    const float uvWidth = textureWidth > 0 ? static_cast<float>(textureWidth) : 1.0f;
    const float uvHeight = textureHeight > 0 ? static_cast<float>(textureHeight) : 1.0f;

    while (headerOffset + 0x30 <= data.size()) {
        const std::uint32_t vertexAddress = readU32(data, headerOffset);
        const std::uint32_t normalAddress = readU32(data, headerOffset + 4);
        const std::uint32_t triangleAddress = readU32(data, headerOffset + 8);
        const std::uint32_t quadAddress = readU32(data, headerOffset + 12);
        if (vertexAddress < loadAddress || normalAddress < loadAddress ||
            triangleAddress < loadAddress || quadAddress < loadAddress) {
            break;
        }

        const std::size_t vertexOffset = vertexAddress - loadAddress;
        const std::size_t normalOffset = normalAddress - loadAddress;
        const std::size_t triangleOffset = triangleAddress - loadAddress;
        const std::size_t quadOffset = quadAddress - loadAddress;
        const std::size_t triangleCount = readU16(data, headerOffset + 0x10);
        const std::size_t quadCount = readU16(data, headerOffset + 0x12);

        // This identifies the vertex/normal/triangle/quad mesh layout. The
        // later DOOR04 block uses a different, likely animation-related layout.
        if (vertexOffset != headerOffset + 0x30 || normalOffset < vertexOffset ||
            triangleOffset < normalOffset || quadOffset < triangleOffset ||
            (normalOffset - vertexOffset) % 8 != 0 ||
            (triangleOffset - normalOffset) % 8 != 0 ||
            triangleOffset + triangleCount * 12 > data.size() ||
            quadOffset + quadCount * 16 > data.size()) {
            break;
        }

        const std::size_t storedVertexCount = (normalOffset - vertexOffset) / 8;
        const std::size_t normalCount = (triangleOffset - normalOffset) / 8;
        if (storedVertexCount == 0 || normalCount < storedVertexCount) {
            break;
        }

        // The vector span includes one 8-byte trailer. Faces are the reliable
        // source of the usable vertex count: in DOOR04 they reference 0..25,
        // while record 26 is non-geometric metadata.
        std::size_t vertexCount = 0;
        const auto includeFaceIndices = [&](const std::size_t offset, const std::size_t count) {
            for (std::size_t i = 0; i < count; ++i) {
                const std::size_t index = readU16(data, offset + i * 2);
                if (index >= storedVertexCount) {
                    return false;
                }
                vertexCount = std::max(vertexCount, index + 1);
            }
            return true;
        };
        for (std::size_t i = 0; i < triangleCount; ++i) {
            if (!includeFaceIndices(triangleOffset + i * 12, 3)) {
                return false;
            }
        }
        for (std::size_t i = 0; i < quadCount; ++i) {
            if (!includeFaceIndices(quadOffset + i * 16, 4)) {
                return false;
            }
        }
        if (vertexCount == 0) {
            break;
        }

        if (!output.is_open()) {
            output.open(outputName, std::ios::trunc);
            if (!output) {
                return false;
            }
            output << "# Dino Crisis 2 DOOR mesh export\n";
            output << "# Only consecutive mesh headers with the confirmed layout are exported.\n";
            output << "mtllib " << fileNameOnly(outputName + ".mtl") << '\n';
            output << "usemtl door_texture\n";
        }

        output << "o door_mesh_" << fileNameOnly(doorFileName) << objectNumber << '\n';
        for (std::size_t i = 0; i < vertexCount; ++i) {
            const std::size_t offset = vertexOffset + i * 8;
            output << "v " << static_cast<std::int16_t>(readU16(data, offset)) * OBJ_SCALE << ' '
                   << static_cast<std::int16_t>(readU16(data, offset + 2)) * OBJ_SCALE << ' '
                   << static_cast<std::int16_t>(readU16(data, offset + 4)) * OBJ_SCALE << '\n';
        }
        for (std::size_t i = 0; i < vertexCount; ++i) {
            const std::size_t offset = normalOffset + i * 8;
            output << "vn " << static_cast<std::int16_t>(readU16(data, offset)) / 4096.0f << ' '
                   << static_cast<std::int16_t>(readU16(data, offset + 2)) / 4096.0f << ' '
                   << static_cast<std::int16_t>(readU16(data, offset + 4)) / 4096.0f << '\n';
        }

        const auto writeFace = [&](const std::size_t offset, const std::size_t count) {
            std::vector<std::size_t> indices;
            for (std::size_t i = 0; i < count; ++i) {
                // DC2 stores quad vertices as 0,1,2,3 where 2 and 3 are
                // opposite sides of the strip. OBJ needs their perimeter
                // order: 0,1,3,2. Triangles retain their stored order.
                const std::size_t sourceIndex = count == 4 && i >= 2 ? 5 - i : i;
                const std::size_t index = readU16(data, offset + sourceIndex * 2);
                if (index >= vertexCount) {
                    return false;
                }
                indices.push_back(index);
            }
            for (std::size_t i = 0; i < count; ++i) {
                const std::size_t uvOffset = offset + count * 2;
                const std::size_t sourceIndex = count == 3 && rotateTriangleUvs ? (i + 1) % 3 :
                                                (count == 4 && i >= 2 ? 5 - i : i);
                const unsigned int u = data[uvOffset + sourceIndex * 2];
                const unsigned int v = data[uvOffset + sourceIndex * 2 + 1];
                output << "vt " << std::fixed << std::setprecision(6)
                       << static_cast<float>(u) / uvWidth << ' '
                       << 1.0f - static_cast<float>(v) / uvHeight << '\n';
            }
            output << "f";
            for (std::size_t i = 0; i < count; ++i) {
                const std::size_t vertexIndex = vertexBase + indices[i];
                const std::size_t textureIndex = uvBase + i;
                const std::size_t normalIndex = normalBase + indices[i];
                output << ' ' << vertexIndex << '/' << textureIndex << '/' << normalIndex;
            }
            output << '\n';
            uvBase += count;
            return true;
        };

        for (std::size_t i = 0; i < triangleCount; ++i) {
            if (!writeFace(triangleOffset + i * 12, 3)) {
                return false;
            }
        }
        for (std::size_t i = 0; i < quadCount; ++i) {
            if (!writeFace(quadOffset + i * 16, 4)) {
                return false;
            }
        }

        vertexBase += vertexCount;
        normalBase += vertexCount;
        headerOffset = quadOffset + quadCount * 16;
        ++objectNumber;
    }

    return objectNumber > 0;
}

} // namespace

DoorUnpacker::DoorUnpacker(std::string filename) : Unpacker(filename) {}

DoorUnpacker::~DoorUnpacker() {
    for (auto waveFile : WAVE_FILES) {
        delete waveFile;
    }
    WAVE_FILES.clear();
}

/**
 * DOOR*.DAT is a Dino Crisis 2 dummy-header archive. The first 0x800 bytes
 * are 0x20-byte entries; each payload begins on an 0x800-byte boundary.
 */
int DoorUnpacker::unpack() {
    if (!inFile.is_open()) {
        std::cout << "[ERROR] Error opening " << filename << '\n';
        return 1;
    }

    std::vector<DirectoryEntry> entries;
    std::uint32_t payloadOffset = DIRECTORY_SIZE;
    for (std::uint32_t entryOffset = 0; entryOffset < DIRECTORY_SIZE; entryOffset += ENTRY_SIZE) {
        DirectoryEntry entry{};
        inFile.clear();
        inFile.seekg(entryOffset, std::ios::beg);
        inFile.read(reinterpret_cast<char*>(&entry), sizeof(entry));
        if (!inFile) {
            std::cout << "[ERROR] Failed to read DOOR archive directory.\n";
            return 1;
        }
        if (std::memcmp(&entry.type, DUMMY_HEADER, sizeof(DUMMY_HEADER) - 1) == 0) {
            break;
        }

        entries.push_back(entry);
        std::cout << "[DEBUG] Entry " << entries.size() - 1
                  << ": type=" << entry.type
                  << ", size=0x" << std::hex << entry.size
                  << ", file offset=0x" << payloadOffset << std::dec << '\n';
        payloadOffset += alignToSector(entry.size);
    }

    if (entries.empty()) {
        std::cout << "[ERROR] No entries found in DOOR archive.\n";
        return 1;
    }

    payloadOffset = DIRECTORY_SIZE;
    const DirectoryEntry* bitmapEntry = nullptr;
    const DirectoryEntry* paletteEntry = nullptr;
    const DirectoryEntry* modelEntry = nullptr;
    std::uint32_t bitmapOffset = 0;
    std::uint32_t paletteOffset = 0;
    std::uint32_t modelOffset = 0;
    for (const DirectoryEntry& entry : entries) {
        if (entry.type == 1 && bitmapEntry == nullptr) {
            bitmapEntry = &entry;
            bitmapOffset = payloadOffset;
        } else if (entry.type == 2 && paletteEntry == nullptr) {
            paletteEntry = &entry;
            paletteOffset = payloadOffset;
        } else if ((entry.type == 5 || (entry.type == 7 && (entry.address & 0x80000000u))) &&
                   modelEntry == nullptr) {
            // The model block is type 5 on PC and type 7 (loaded at 0x80xxxxxx) on PlayStation.
            modelEntry = &entry;
            modelOffset = payloadOffset;
        }
        payloadOffset += alignToSector(entry.size);
    }

    if (modelEntry == nullptr) {
        std::cout << "[ERROR] No model block found in DOOR archive.\n";
        return 1;
    }

    std::vector<unsigned char> model(modelEntry->size);
    if (!readAt(inFile, modelOffset, model)) {
        std::cout << "[ERROR] Failed to read model candidate.\n";
        return 1;
    }
    std::vector<unsigned char> decompressedModel;
    bool exportedObj = false;
    if (!decompressDc2Lzss(model, decompressedModel)) {
        std::cout << "[WARNING] Model block did not decode as DC2 LZSS; skipping model export.\n";
    } else {
        const int textureWidth = bitmapEntry == nullptr ? 0 : static_cast<int>((bitmapEntry->reserved & 0xffff) * 2);
        const int textureHeight = bitmapEntry == nullptr ? 0 : static_cast<int>(bitmapEntry->reserved >> 16);
        exportedObj = exportDc2MeshesToObj(
                decompressedModel,
                modelEntry->address,
                textureWidth,
                textureHeight,
                filename + ".data.model.uvshift.obj",
                filename,
                true);
        if (exportedObj) {
            std::cout << "[INFO] Saved UV-shift experiment to " << filename
                      << ".data.model.uvshift.obj\n";
        }
    }

    // Type 1 is an 8-bit indexed texture; type 2 is its 256-colour RGB555 CLUT.
    if (bitmapEntry != nullptr && paletteEntry != nullptr) {
        std::vector<unsigned char> texture(bitmapEntry->size);
        std::vector<unsigned char> texturePalette(paletteEntry->size);
        if (!readAt(inFile, bitmapOffset, texture) || !readAt(inFile, paletteOffset, texturePalette)) {
            std::cout << "[WARNING] Failed to read indexed texture preview.\n";
        } else {
            const int textureWidth = static_cast<int>((bitmapEntry->reserved & 0xffff) * 2);
            const int textureHeight = static_cast<int>(bitmapEntry->reserved >> 16);
            const std::vector<unsigned char> textureRgb888 = converter->convert(texturePalette);
            QVector<QRgb> colorTable;
            for (std::size_t i = 0; i < textureRgb888.size(); i += 3) {
                colorTable.push_back(QColor(
                    textureRgb888[i], textureRgb888[i + 1], textureRgb888[i + 2]
                ).rgb());
            }
            const bool is4BitTexture = texturePalette.size() <= 0x20;
            // Texture tiles are stored two across and eight down.  Standard
            // doors use 8-bit 64x32-pixel tiles; the small DOOR1900 variant
            // uses 4-bit 64x16-pixel tiles.
            const std::size_t tileRowBytes = is4BitTexture ? 32 : 64;
            const std::size_t tileRows = is4BitTexture ? 16 : 32;
            const std::vector<std::uint8_t> tiledTexture = rearrangeDoorTiles(texture, tileRowBytes, tileRows);
            if (tiledTexture.empty()) {
                std::cout << "[WARNING] Unsupported indexed texture tile layout.\n";
                return 0;
            }
            const int outputWidth = is4BitTexture ? 128 : 128;
            const int outputHeight = static_cast<int>(tileRows * 8);
            std::vector<std::uint8_t> indexedTexture;
            if (is4BitTexture) {
                indexedTexture.reserve(tiledTexture.size() * 2);
                for (const std::uint8_t packed : tiledTexture) {
                    indexedTexture.push_back(packed & 0x0f);
                    indexedTexture.push_back(packed >> 4);
                }
            }
            QImage tiledTextureImage(
                is4BitTexture ? indexedTexture.data() : tiledTexture.data(),
                outputWidth,
                outputHeight,
                QImage::Format::Format_Indexed8
            );
            tiledTextureImage.setColorTable(colorTable);
            if (tiledTextureImage.save(QString::fromStdString(filename + ".texture.tiled.png"), "PNG")) {
                std::cout << "[INFO] Saved tile-reordered texture to " << filename
                          << ".texture.tiled.png\n";
                if (exportedObj) {
                    const std::string objName = filename + ".data.model.uvshift.obj";
                    if (writeObjMaterial(objName, filename + ".texture.tiled.png")) {
                        std::cout << "[INFO] Linked texture material to " << objName << '\n';
                    } else {
                        std::cout << "[WARNING] Failed to write OBJ material file.\n";
                    }
                }
            } else {
                std::cout << "[WARNING] Failed to save tile-reordered texture.\n";
            }
        }
    }

    return 0;
}

std::string DoorUnpacker::getName() {
    return "DoorUnpacker";
}
