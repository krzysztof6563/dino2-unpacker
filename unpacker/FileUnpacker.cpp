#include "FileUnpacker.h"

FileUnpacker::FileUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->PNG_WIDTH = 128;
    this->PNG_HEIGHT = 96;
}

int FileUnpacker::unpack() {
    int numberOfchunks = this->dechunker->getNumberOfChunks();
    this->extractImages(numberOfchunks / 7, 6, 0, 2, 4, 0);

    return 0;
}

std::string FileUnpacker::getName() {
    return "FileUnpacker";
}