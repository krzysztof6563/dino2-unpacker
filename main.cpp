#include <iostream>
#include <fstream>
#include <string.h>
#include "unpacker/itemunpacker.h"
#include "unpacker/doorunpacker.h"


using namespace std;

Unpacker *getUnpackerByFilename(std::string filename) {
    if (filename == "ITEM.DAT") {
        return new ItemUnpacker(filename);
    } else if (filename.find("DOOR") != std::string::npos) {
        return new DoorUnpacker(filename);
    }
    return nullptr;
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
            } else {
                std::cout << "File type not supported" << std::endl;
            }
            delete u;
        }

    }
    return 0;
    }
