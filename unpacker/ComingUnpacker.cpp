#include "ComingUnpacker.h"

ComingUnpacker::ComingUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->loadChunksToRGB555Vector();
    this->PNG_WIDTH = 640;
    this->PNG_WIDTH = 64*10;
    this->PNG_HEIGHT = 480;
    this->PNG_HEIGHT = 32*15;
}

int ComingUnpacker::unpack() {
    auto palette = this->dechunker->getChunkAt(this->dechunker->getNumberOfChunks()-1);
    for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
        paletteData.push_back(palette[j]);
    }
    paletteData = this->converter->convert(paletteData);

    this->rgb555Data = this->rearrangeChunks(10, 15, 1, this->rgb555Data);
    this->saveAsIndexedPNG(this->filename);

    return 1;
}

std::string ComingUnpacker::getName() {
    return "ComingUnpacker";
}