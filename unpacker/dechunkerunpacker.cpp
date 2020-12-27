#include "dechunkerunpacker.h"

DechunkerUnpacker::DechunkerUnpacker(std::string filename) : Unpacker (filename)
{
    std::cout << "** Unpacker type: DechunkerUnpacker" << std::endl;
}

int DechunkerUnpacker::unpack()
{
    this->dechunker->dechunk();
    this->dechunker->saveChunksToDisk();
    return 1;
}

