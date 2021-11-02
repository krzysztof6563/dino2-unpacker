#ifndef ARMORUNPACKER_H
#define ARMORUNPACKER_H

#include "unpacker.h"
#include <string>

class ArmorUnpacker : public Unpacker {
    public:
        ArmorUnpacker(std::string filename);

        int unpack();
        std::string getName();
};

#endif // ARMORUNPACKER_H
