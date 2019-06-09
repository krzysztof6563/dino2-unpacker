#ifndef PXLUNPACKER_H
#define PXLUNPACKER_H

#include "unpacker.h"
#include <string>

class PXLUnpacker : public Unpacker
{
public:
    PXLUnpacker(std::string filename);

    int unpack();
};

#endif // PXLUNPACKER_H
