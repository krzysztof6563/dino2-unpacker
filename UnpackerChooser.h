#pragma once

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

class UnpackerChooser {
    public:
        static Unpacker *getUnpackerByFilename(std::string filename) {
            if (filename == "ITEM.DAT") {
                return new ItemUnpacker(filename);
            } else if (filename == "COMING.DAT") {
                return new ComingUnpacker(filename);
            } else if (filename == "FILE.DAT") {
                return new FileUnpacker(filename);
            } else if (filename.find("DOOR") != std::string::npos) {
                return new DoorUnpacker(filename);
            } else if (filename.find("ARMOR") != std::string::npos ||
                    filename.find("CAPLOGO.DAT") != std::string::npos ||
                    filename.find("MAP.BIN") != std::string::npos) {
                return new ArmorUnpacker(filename);
            } else if (filename.find(".PXL") != std::string::npos) {
                return new PXLUnpacker(filename);
            } else if (filename.find("WP") != std::string::npos) {
                return new WPUnpacker(filename);
            } else if (filename.find(".TEX") != std::string::npos) {
                return new TEXUnpacker(filename);
            } else if (filename.find(".DBS") != std::string::npos) {
                return new StageDBSUnpacker(filename);
            } else if (filename.find("SUBSCR6.DAT") != std::string::npos) {
                return new Subscr6Unpacker(filename);
            } else if (filename.find("ENTRY.DAT") != std::string::npos) {
                return new EntryUnpacker(filename);
            } else if (filename.find("WEP_") != std::string::npos) {
                return new WEPUnpacker(filename);
            } else {
                return new DechunkerUnpacker(filename);
            }
        //    return nullptr;
        }
};
