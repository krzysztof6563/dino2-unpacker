#ifndef DECHUNKERUNPACKER_H
#define DECHUNKERUNPACKER_H

#include "unpacker.h"
#include <string>

class DechunkerUnpacker : public Unpacker
{
public:
    DechunkerUnpacker(std::string filename);

    int unpack();
};

#endif // DECHUNKERUNPACKER_H
