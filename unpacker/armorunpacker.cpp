#include "armorunpacker.h"

ArmorUnpacker::ArmorUnpacker(std::string filename) : Unpacker (filename) {
}

int ArmorUnpacker::unpack()
{
    dechunker->dechunk();
    size_t length = dechunker->getNumberOfChunks();
    outFile.open(filename+".out", std::ios::out | std::ios::binary | std::ios::trunc);
    for (size_t i=1;i<length;i++) {
        char *chunk = dechunker->getChunkAt(i);
        outFile.write(chunk, dechunker->getChunkSize());
    }
    outFile.flush();
    outFile.close();
    return 0;
}
