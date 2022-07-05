#include "EntryUnpacker.h"

EntryUnpacker::EntryUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->PNG_WIDTH = 64;
    this->PNG_HEIGHT = 64 ;
}

int EntryUnpacker::unpack() {
    this->extractImages(12, 2);
   
    return 0;
}

std::string EntryUnpacker::getName() {
    return "EntryUnpacker";
}