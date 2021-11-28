#include "FileUnpacker.h"

FileUnpacker::FileUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->PNG_WIDTH = 128;
    this->PNG_HEIGHT = 96;
}

int FileUnpacker::unpack() {
    int numberOfchunks = this->dechunker->getNumberOfChunks();
    int numberOfImages = numberOfchunks / 7;

    if (!std::filesystem::is_directory(filename+"_pngs")){
        std::filesystem::create_directory(filename+"_pngs");
    }

    for (size_t i = 0; i < numberOfImages; i++)
    {
        this->rgb555Data.clear();
        this->paletteData.clear();
        for (size_t j = 0; j < 6; j+=2)
        {
            char* chunk = dechunker->getChunkAt(i*7 + j);
            char* chunk2 = dechunker->getChunkAt(i*7 + j + 1);

            for (size_t k = 0; k < 64; k++)
            {
                char* activeChunk = chunk;
                if (k%2 !=0) {
                    activeChunk = chunk2;
                }
                for (size_t l = 0; l < 64; l++) {
                    this->rgb555Data.push_back(activeChunk[k/2*64 + l]);
                }
            }
        }

        char* palletteChunk = this->dechunker->getChunkAt(i*7 + 6);
        for (size_t k = 0; k < this->dechunker->getChunkSize(); k++) {
            this->paletteData.push_back(palletteChunk[k]);
        }
        paletteData = this->converter->convert(paletteData);

        this->saveAsIndexedPNG(this->filename + "_pngs/" + std::to_string(i));
    }

    return 0;
}

std::string FileUnpacker::getName() {
    return "FileUnpacker";
}