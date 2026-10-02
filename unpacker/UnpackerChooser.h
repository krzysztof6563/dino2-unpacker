#pragma once

#include <filesystem>

#include "unpacker/ItemUnpacker.h"
#include "unpacker/DoorUnpacker.h"
#include "unpacker/DechunkerUnpacker.h"
#include "unpacker/ArmorUnpacker.h"
#include "unpacker/PXLUnpacker.h"
#include "unpacker/WPUnpacker.h"
#include "unpacker/TEXUnpacker.h"
#include "unpacker/StageDBSUnpacker.h"
#include "unpacker/FileUnpacker.h"
#include "unpacker/ComingUnpacker.h"
#include "unpacker/Subscr6Unpacker.h"
#include "unpacker/EntryUnpacker.h"
#include "unpacker/WEPUnpacker.h"
#include "unpacker/ModelUnpacker.h"

class UnpackerChooser {
    public:
        static bool isEnemyModelFile(const std::string& filename) {
            constexpr std::size_t ENEMY_FILE_NAME_LENGTH = 7; // E00.DAT
            constexpr std::size_t EXTENSION_OFFSET = 3;
            constexpr const char* DAT_EXTENSION = ".DAT";

            return filename.size() == ENEMY_FILE_NAME_LENGTH &&
                   filename.front() == 'E' &&
                   filename.substr(EXTENSION_OFFSET) == DAT_EXTENSION;
        }

        static Unpacker *getUnpackerByFilename(std::string filename) {
            std::string base = std::filesystem::path(filename).filename().string();
            if (base == "ITEM.DAT") {
                return new ItemUnpacker(filename);
            } else if (base == "COMING.DAT") {
                return new ComingUnpacker(filename);
            } else if (base == "FILE.DAT") {
                return new FileUnpacker(filename);
            } else if (base.find("DOOR") != std::string::npos) {
                return new DoorUnpacker(filename);
            } else if (base.find("ARMOR") != std::string::npos ||
                    base.find("CAPLOGO.DAT") != std::string::npos ||
                    base.find("MAP.BIN") != std::string::npos) {
                return new ArmorUnpacker(filename);
            } else if (base.find(".PXL") != std::string::npos) {
                return new PXLUnpacker(filename);
            } else if (base.find("WP") != std::string::npos) {
                return new WPUnpacker(filename);
            } else if (base.find(".TEX") != std::string::npos) {
                return new TEXUnpacker(filename);
            } else if (base.find(".DBS") != std::string::npos) {
                return new StageDBSUnpacker(filename);
            } else if (base.find("SUBSCR6.DAT") != std::string::npos) {
                return new Subscr6Unpacker(filename);
            } else if (base.find("ENTRY.DAT") != std::string::npos) {
                return new EntryUnpacker(filename);
            } else if (base.find("WEP_") != std::string::npos) {
                return new WEPUnpacker(filename);
            } else if (base == "CORE.DAT" || base == "RESULT.DAT" ||
                       isEnemyModelFile(base)) {
                return new ModelUnpacker(filename);
            } else {
                return new DechunkerUnpacker(filename);
            }
        //    return nullptr;
        }
};
