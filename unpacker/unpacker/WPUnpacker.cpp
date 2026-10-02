#include "WPUnpacker.h"

WPUnpacker::WPUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    
}

int WPUnpacker::unpack() {
    size_t totalChunks = this->dechunker->getNumberOfChunks(); 
    this->PNG_HEIGHT = (this->dechunker->getNumberOfChunks()-2)*32;
    this->extractImages(1, this->PALLETTE_CHUNK - 1, 1);

    return 1;
}

std::string WPUnpacker::getName() {
    return "WPUnpacker";
}