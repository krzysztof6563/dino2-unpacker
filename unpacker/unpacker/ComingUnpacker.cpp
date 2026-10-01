#include "ComingUnpacker.h"

ComingUnpacker::ComingUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->loadChunksToRGB555Vector();
    this->PNG_WIDTH = 64*10;
    this->PNG_HEIGHT = 32*15;
}

int ComingUnpacker::unpack() {
    this->extractImages(1, this->dechunker->getNumberOfChunks()-1, 0, 10, 15, 1);

    return 1;
}

std::string ComingUnpacker::getName() {
    return "ComingUnpacker";
}