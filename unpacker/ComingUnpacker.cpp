#include "ComingUnpacker.h"

ComingUnpacker::ComingUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->loadChunksToRGB555Vector();
    this->PNG_WIDTH = 640;
    this->PNG_HEIGHT = 480;
}

int ComingUnpacker::unpack() {
    auto palette = this->dechunker->getChunkAt(this->dechunker->getNumberOfChunks()-1);
    for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
        paletteData.push_back(palette[j]);
    }
    paletteData = this->converter->convert(paletteData);

    std::vector<uint8_t> newData;

    int chunkSize = this->dechunker->getChunkSize();

    for (size_t i = 1; i < this->dechunker->getNumberOfChunks() - 2; i += 10) { //rows of chunks for whole image
        for (size_t j = 0; j < 32; j++) // rows for chunk height
        {
            for (size_t k = 0; k < 10; k++) //loop over chunks in a row
            {
                int chunkStart = (i + k) * chunkSize;
                int offset = (64 * j);

                std::cout << "Chunk: " << chunkStart / this->dechunker->getChunkSize() 
                          << " Row: " << j
                          << " Saving at: " << newData.size() << std::endl; 

                // for (size_t l = 0; l < 64; l++)
                // {
                    
                //     newData.push_back(this->rgb555Data.at(chunkStart + offset + l));
                // }
                    
                //copy row of pixels
                std::copy(
                    rgb555Data.begin() + chunkStart + offset, 
                    rgb555Data.begin() + chunkStart + offset + 64, 
                    std::back_inserter(newData)
                );
            }
        }
    }

    this->rgb555Data = newData;

    std::cout << "SIZE: " << this->rgb555Data.size() << ". Should be " << 640*480 << std::endl;

    this->saveAsIndexedPNG(this->filename, this->dechunker->getChunkSize());

    return 1;
}

std::string ComingUnpacker::getName() {
    return "ComingUnpacker";
}