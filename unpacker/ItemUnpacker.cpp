#include "ItemUnpacker.h"

ItemUnpacker::ItemUnpacker(std::string filename) : Unpacker (filename) {
    this->PNG_WIDTH = 128;
    this->PNG_HEIGHT = 64;
}

ItemUnpacker::~ItemUnpacker() {
}

int ItemUnpacker::unpack() {
    if (inFile.is_open()) {
        std::cout << "File" << filename << " opened." << std::endl;
        this->dechunker->dechunk();
        size_t total = dechunker->getNumberOfChunks();
        int imageNumber = 0;
        if (!std::filesystem::is_directory("ITEM_pngs")){
            std::filesystem::create_directory("ITEM_pngs");
        }
        for (size_t i = 0; i < total; i += 5, imageNumber++) {
            rgb555Data.clear();

            for (size_t j = 0; j < 4; j++) {
                auto chunk = dechunker->getChunkAt(i+j);
                outFile.write(chunk, dechunker->getChunkSize());
                for (size_t k = 0; k < this->dechunker->getChunkSize(); k++) {
                    this->rgb555Data.push_back(chunk[k]);
                }
            }

            this->rgb555Data = this->rearrangeChunks(2, 2, 0, this->rgb555Data);

            char* chunk = dechunker->getChunkAt(i+4);
            this->paletteData.clear();
            for (size_t i = 0; i < dechunker->getChunkSize(); i++)
            {
                this->paletteData.push_back(chunk[i]);
            }

            paletteData = this->converter->convert(paletteData);
            this->saveAsIndexedPNG("ITEM_pngs/" + this->filename + std::to_string(imageNumber));
        }
        inFile.close();

    } else {
        std::cout << "Error opening  " << filename;
    }
    return 0;
}

std::string ItemUnpacker::getName() {
    return "ItemUnpacker";
}