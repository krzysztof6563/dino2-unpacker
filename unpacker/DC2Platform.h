#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Dino Crisis 2 shipped on PC and PlayStation with the same file names and the same DAT
// container: a 0x800-byte table of 0x20-byte entries (u32 type, size, load address,
// extra), then each payload padded to 0x800 bytes. What differs is inside:
//
//   content                      PC                          PlayStation
//   model + animation block      type 5, loaded at 0x6xxxxx   type 7, loaded at 0x80xxxxxx
//   character program code       -                           extra type-7 blocks
//   compressed texture           type 6                      type 8
//   sounds                       type 3: RIFF WAV bank       type 3 "Gian" header + type 4
//                                                            SPU ADPCM samples
//   room backgrounds (.DBS)      JPEG                        MDEC bitstream ("BS" frames)
//
// Texture-only files (.TEX, .PXL, ENTRY, COMING, SC*, ITEM ...) are identical on both.
namespace dc2 {

enum class GameVersion { Unknown, PC, PlayStation };

struct ContainerEntry {
    std::uint32_t type = 0, size = 0, address = 0, extra = 0;
    std::uint64_t offset = 0;      // payload position in the file
};

// Reads a DAT container's entry table. Returns false when the file is not a container
// (or the table points past the end of the file).
bool readContainer(const std::string& filename, std::vector<ContainerEntry>& entries);

// Which version of the game a file comes from, or Unknown when its format is the same on
// both (or not recognised).
GameVersion detectVersion(const std::string& filename);

const char* versionName(GameVersion version);

}
