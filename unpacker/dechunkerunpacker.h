#ifndef DECHUNKERUNPACKER_H
#define DECHUNKERUNPACKER_H

#include "unpacker.h"
#include <string>

class DechunkerUnpacker : public Unpacker
{
public:
    DechunkerUnpacker(std::string filename);

    int unpack();
    std::string unpackerName = "DechunkerUpdater";

};

#endif // DECHUNKERUNPACKER_H
