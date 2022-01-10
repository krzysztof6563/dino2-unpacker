#include "Subscr6Unpacker.h"

Subscr6Unpacker::Subscr6Unpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->loadChunksToRGB555Vector();
}

int Subscr6Unpacker::unpack() {
    auto palette = this->dechunker->getChunkAt(this->PALLETTE_CHUNK);
    for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
        paletteData.push_back(palette[j]);
    }
    paletteData = this->converter->convert(paletteData);

    this->PNG_HEIGHT = (this->dechunker->getNumberOfChunks()-2)*32;
    this->saveAsIndexedPNG(this->filename, this->dechunker->getChunkSize());

    return 1;
}

std::string Subscr6Unpacker::getName() {
    return "Subscr6Unpacker";
}