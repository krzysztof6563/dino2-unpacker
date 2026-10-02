#include "WPUnpacker.h"

WPUnpacker::WPUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    
}

int WPUnpacker::unpack() {
    size_t totalChunks = this->dechunker->getNumberOfChunks();
    // The PlayStation version also has small WP*A.DAT files that hold only weapon program
    // code, no image.
    if (totalChunks < static_cast<size_t>(PALLETTE_CHUNK) + 1) {
        std::cout << "[INFO] No image in this file (PlayStation weapon program code)\n";
        return 0;
    }
    this->PNG_HEIGHT = (this->dechunker->getNumberOfChunks()-2)*32;
    this->extractImages(1, this->PALLETTE_CHUNK - 1, 1);

    return 1;
}

std::string WPUnpacker::getName() {
    return "WPUnpacker";
}