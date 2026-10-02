#include "FileUnpacker.h"

FileUnpacker::FileUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->PNG_WIDTH = 128;
    this->PNG_HEIGHT = 96;
}

int FileUnpacker::unpack() {
    int numberOfchunks = this->dechunker->getNumberOfChunks();
    // Each Dino File image is 128x96: 2x3 tiles of 64x32 (6 chunks), then its palette chunk.
    this->extractImages(numberOfchunks / 7, 6, 0, 2, 3, 0);

    return 0;
}

std::string FileUnpacker::getName() {
    return "FileUnpacker";
}