#include "DechunkerUnpacker.h"

DechunkerUnpacker::DechunkerUnpacker(std::string filename) : Unpacker (filename) {
}

int DechunkerUnpacker::unpack() {
    this->dechunker->dechunk();
    this->dechunker->saveChunksToDisk();
    return 1;
}

std::string DechunkerUnpacker::getName() {
    return "DechunkerUnpacker";
}