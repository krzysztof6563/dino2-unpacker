#include "TEXUnpacker.h"

TEXUnpacker::TEXUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->loadChunksToRGB555Vector();
}

int TEXUnpacker::unpack() {
    int numberOfchunks = this->dechunker->getNumberOfChunks()-1;
    int imageHeight = numberOfchunks == 9 ? 8*32 : (34-2)*32;
    int paletteIndex = numberOfchunks == 9 ? 9 : 34-1;
    auto palette = this->dechunker->getChunkAt(paletteIndex); //34-1 is index of first palette
    
    for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
        paletteData.push_back(palette[j]);
    }
    paletteData = this->converter->convert(paletteData);

    this->PNG_HEIGHT = imageHeight;
    this->saveAsIndexedPNG(this->filename, this->dechunker->getChunkSize());

    if (numberOfchunks == 66) {
        paletteData.clear();
        palette = this->dechunker->getChunkAt(numberOfchunks);
        for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
            paletteData.push_back(palette[j]);
        }
        paletteData = this->converter->convert(paletteData);

        this->saveAsIndexedPNG(this->filename+"_2", (34)*this->dechunker->getChunkSize());
    }
    return 1;
}

std::string TEXUnpacker::getName() {
    return "TEXUnpacker";
}