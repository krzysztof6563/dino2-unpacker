#include <iostream>
#include <fstream>
#include <string.h>
#include "unpacker/itemunpacker.h"
#include "unpacker/doorunpacker.h"
#include "unpacker/dechunkerunpacker.h"
#include "unpacker/armorunpacker.h"
#include "unpacker/pxlunpacker.h"
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
    } else {
        return new DechunkerUnpacker(filename);
    }
//    return nullptr;
}

//void reshapeImage(char buffer[]) {
//    char newBuffer[BUFFER_SIZE];
//    for (int i=0;i<TILE_SIZE;i++) {
//        newBuffer[i] = buffer[i];
//        newBuffer[i+TILE_WIDTH] = buffer[TILE_SIZE+1];
//        newBuffer[i+2*TILE_SIZE] = buffer[i+2*TILE_SIZE];
//        newBuffer[i+TILE_WIDTH+2*TILE_SIZE] = buffer[TILE_SIZE+1+2*TILE_SIZE];
//    }
//    for (int i=0;i<BUFFER_SIZE;i++) {
//        buffer[i] = newBuffer[i];
//    }
//    return;
//}

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
