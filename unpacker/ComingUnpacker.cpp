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

    std::vector<uint8_t> newData;
    newData.reserve(640*480);

    int chunkSize = this->dechunker->getChunkSize();

    int stop = dechunker->getNumberOfChunks() - 2;

    int wc = 0;

    for (size_t i = 1; i < stop; i += 10) { //rows of chunks for whole image
        for (size_t j = 0; j < 32; j++) // rows for chunk height
        {
            for (size_t k = 0; k < 10; k++) //loop over chunks in a row
            {
                int chunkStart = (i + k) * chunkSize;
                int offset = (64 * j);
                std::copy(
                    rgb555Data.begin() + chunkStart + offset, 
                    rgb555Data.begin() + chunkStart + offset + 64, 
                    std::back_inserter(newData)
                );
            }
        }
    }

    this->rgb555Data = newData;


    this->saveAsIndexedPNG(this->filename);

    return 1;
}

std::string ComingUnpacker::getName() {
    return "ComingUnpacker";
}