#include <iostream>
#include <fstream>
#include <string.h>
#include "unpacker/ItemUnpacker.h"
#include "unpacker/doorunpacker.h"
#include "unpacker/dechunkerunpacker.h"
#include "unpacker/armorunpacker.h"
#include "unpacker/PXLUnpacker.h"
#include "unpacker/WPUnpacker.h"
#include "dechunker.h"

using namespace std;

Unpacker *getUnpackerByFilename(std::string filename) {
    if (filename == "ITEM.DAT") {
        return new ItemUnpacker(filename);
    } else if (filename.find("DOOR") != std::string::npos) {
        return new DoorUnpacker(filename);
    } else if (filename.find("ARMOR") != std::string::npos ||
               filename.find("CAPLOGO.DAT") != std::string::npos ||
               filename.find(".TEX") != std::string::npos ||
               filename.find("MAP.BIN") != std::string::npos ||
               filename.find("MS_") != std::string::npos ) {
        return new ArmorUnpacker(filename);
    } else if (filename.find(".PXL") != std::string::npos) {
        return new PXLUnpacker(filename);
    } else if (filename.find("WP") != std::string::npos) {
        return new WPUnpacker(filename);
    } else {
        return new DechunkerUnpacker(filename);
    }
//    return nullptr;
}

int main(int argc, char *argv[]){
    if (argc > 1){
        for (int i=1; i<argc; i++) {
            std::string filename(argv[i]);
            Unpacker *u = getUnpackerByFilename(filename);
            if (u != nullptr) {
                u->unpack();
                std::cout << "Unpacked: " << filename << std::endl;
            } else {
                std::cout << "File type not supported" << std::endl;
            }
            delete u;
        }

    }
    return 0;
    }
