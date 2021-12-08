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
        for (size_t j = 0; j < 6; j++) {
            char* chunk = dechunker->getChunkAt(i*7 + j);
            for (size_t k = 0; k < this->dechunker->getChunkSize(); k++) {
                this->rgb555Data.push_back(chunk[k]);
            }
        }

        this->rgb555Data = this->rearrangeChunks(2, 4, 0, this->rgb555Data);

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