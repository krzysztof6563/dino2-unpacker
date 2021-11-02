#include <iostream>
#include <fstream>
#include <string.h>
#include "unpacker/ItemUnpacker.h"
#include "unpacker/DoorUnpacker.h"
#include "unpacker/DechunkerUnpacker.h"
#include "unpacker/ArmorUnpacker.h"
#include "unpacker/PXLUnpacker.h"
#include "unpacker/WPUnpacker.h"
#include "unpacker/TEXUnpacker.h"
#include "unpacker/StageDBSUnpacker.h"
#include "dechunker.h"

using namespace std;

void displayHelp() {
    std::cout << R"EOL(
    Usage: dino2-unpacker [OPTIONS] FILE...

    Supported file formats:

    |---------------------------------------------------------------------------|
    | FILE PATTERN | SUPPORTED  | NOTES                                          |
    | ------------ | ---------  | ---------------------------------------------- |
    | *.PXL        | ✅         | Saves images as PNGs                           |
    | *.TEX        | ✅         | Saves images as PNGs                           |
    | ARMOR*.DAT   | 🔨         | Implemented dechunking                         |
    | CAPLOGO.DAT  | ❌         |                                                |
    | COMING.DAT   | ❌         |                                                |
    | CONTINUE.DAT | ❌         |                                                |
    | CORE*.DAT    | ❌         |                                                |
    | DEMO*.TRG    | ❌         |                                                |
    | DOOR*.DAT    | 🔨         | Extracts textures and soudns, 3D model missing |
    | E*.DAT       | ❌         |                                                |
    | ENDING.DAT   | ❌         |                                                |
    | ENTRY.DAT    | ❌         |                                                |
    | FILE.DAT     | ❌         |                                                |
    | GAMEOVER.DAT | ❌         |                                                |
    | ITEM.DAT     | ✅         | Saves images as PNGs                           |
    | KOF_*.DAT    | ❌         |                                                |
    | LOAD.DAT     | ❌         |                                                |
    | MAP.BIN      | ❌         |                                                |
    | MAP.DAT      | ❌         |                                                |
    | MAP_ST*.DAT  | ❌         |                                                |
    | ME_*.DAT     | ❌         |                                                |
    | MF_*.DAT     | ❌         |                                                |
    | M_RESULT.DAT | ❌         |                                                |
    | MS_*.DAT     | ❌         |                                                |
    | M_TITLE.DAT  | ❌         |                                                |
    | NO_DATA.PXL  | ❌         |                                                |
    | OPENING.DAT  | ❌         |                                                |
    | OPTION.DAT   | ❌         |                                                |
    | RES*.DAT     | ❌         |                                                |
    | RESULT.DAT   | ❌         |                                                |
    | SAVE.DAT     | ❌         |                                                |
    | SC*.DAT      | ❌         |                                                |
    | ST*.DAT      | ❌         |                                                |
    | ST*.DBS      | ✅         | Saves invidual iamges as PNGs                  |
    | SUBSCR3.DAT  | ❌         |                                                |
    | SUBSCR6.DAT  | ❌         |                                                |
    | TITLE.DAT    | ❌         |                                                |
    | TITLE2.DAT   | ❌         |                                                |
    | WARNING.DAT  | ❌         |                                                |
    | WEP_*.DAT    | ❌         |                                                |
    | WP*.DAT      | ✅         | Saves images as PNGs                           |
    ------------------------------------------------------------------------------

    Copyright Krzysztof Michalski 2019 - 2021
    https://github.com/krzysztof6563/dino2-unpacker

    Licensed under GPL-3.0 License
    https://www.gnu.org/licenses/gpl-3.0.html
    )EOL";

}

Unpacker *getUnpackerByFilename(std::string filename) {
    if (filename == "ITEM.DAT") {
        return new ItemUnpacker(filename);
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
    } else {
        return new DechunkerUnpacker(filename);
    }
//    return nullptr;
}

int main(int argc, char *argv[]) {
    if (argc > 1){
        for (int i=1; i<argc; i++) {
            std::string filename(argv[i]);
            Unpacker *u = getUnpackerByFilename(filename);
            if (u != nullptr) {
                std::cout << "[INFO] Choosen Unpacker type: " << u->getName() << std::endl;
                u->unpack();
                std::cout << "[INFO] Unpacked: " << filename << std::endl;
            } else {
                std::cout << "[ERROR] File type not supported" << std::endl;
            }
            delete u;
        }
    } else {
        displayHelp();
    }
    return 0;
    }
