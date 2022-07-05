#include "ItemUnpacker.h"

ItemUnpacker::ItemUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->PNG_WIDTH = 128;
    this->PNG_HEIGHT = 64;
}

ItemUnpacker::~ItemUnpacker() {
}

int ItemUnpacker::unpack() {
    size_t totalChunks = dechunker->getNumberOfChunks();
    this->extractImages(totalChunks / 5, 4, 0, 2, 2, 0);
    
    return 0;
}

std::string ItemUnpacker::getName() {
    return "ItemUnpacker";
}