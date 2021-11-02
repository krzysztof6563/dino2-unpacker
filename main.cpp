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

    ---------------------------------------------------
    | FILE PATTERN | SUPPORTED | NOTES                |
    ---------------------------------------------------
    | *.PXL        | YES       |                      |
    | *.TEX        | YES       |                      |
    | ARMOR*.DAT   | YES       |                      |
    | CAPLOGO.DAT  | NO        |                      |
    | COMING.DAT   | NO        |                      |
    | CONTINUE.DAT | NO        |                      |
    | CORE*.DAT    | NO        |                      |
    | DEMO*.TRG    | NO        |                      |
    | DOOR*.DAT    | ALMOST    | No 3D model data     |
    | E*.DAT       | NO        |                      |
    | ENDING.DAT   | NO        |                      |
    | ENTRY.DAT    | NO        |                      |
    | FILE.DAT     | NO        |                      |
    | GAMEOVER.DAT | NO        |                      |
    | ITEM.DAT     | YES       | No 3D model data     |
    | KOF_P00P.DAT | NO        |                      |
    | LOAD.DAT     | NO        |                      |
    | MAP.BIN      | NO        |                      |
    | MAP.DAT      | NO        |                      |
    | MAP_ST0.DAT  | NO        |                      |
    | ME_0A00.DAT  | NO        |                      |
    | MF_0A00.DAT  | NO        |                      |
    | M_RESULT.DAT | NO        |                      |
    | MS_0000.DAT  | NO        |                      |
    | M_TITLE.DAT  | NO        |                      |
    | NO_DATA.PXL  | NO        |                      |
    | OPENING.DAT  | NO        |                      |
    | OPTION.DAT   | NO        |                      |
    | RES00.DAT    | NO        |                      |
    | RESULT.DAT   | NO        |                      |
    | SAVE.DAT     | NO        |                      |
    | SC01.DAT     | NO        |                      |
    | ST*.DAT      | NO        |                      |
    | ST*.DBS      | YES       |                      |
    | SUBSCR3.DAT  | NO        |                      |
    | SUBSCR6.DAT  | NO        |                      |
    | TITLE.DAT    | NO        |                      |
    | TITLE2.DAT   | NO        |                      |
    | WARNING.DAT  | NO        |                      |
    | WEP_P000.DAT | NO        |                      |
    | WP*.DAT      | YES       |                      |
    ---------------------------------------------------

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
