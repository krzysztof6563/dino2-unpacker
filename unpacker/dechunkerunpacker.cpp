#include "dechunkerunpacker.h"

DechunkerUnpacker::DechunkerUnpacker(std::string filename) : Unpacker (filename)
{
}

int DechunkerUnpacker::unpack()
{
    this->dechunker->dechunk();
    this->dechunker->saveChunksToDisk();
    return 1;
}

