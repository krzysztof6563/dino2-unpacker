#include "PXLUnpacker.h"

PXLUnpacker::PXLUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->loadChunksToRGB555Vector();
    this->PNG_WIDTH = 320;
    this->PNG_HEIGHT = 240;
}

int PXLUnpacker::unpack() {
    this->convertToRGB888();
    this->saveAsRGB888PNG(this->filename);
    std::cout << "Saved as PNG file named: " << this->filename << ".png" << '\n'; 

    return 1;
}

std::string PXLUnpacker::getName() {
    return "PXLUnpacker";
}