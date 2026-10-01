#ifndef WEPUnpacker_H
#define WEPUnpacker_H

#include "unpacker.h"
#include <string>
#include <vector>
#include <cmath>

class WEPUnpacker : public Unpacker {
    public:
        WEPUnpacker(std::string filename);

        int unpack();
        std::string getName();

        bool isConverted = false; 
};

#endif // WEPUnpacker_H
