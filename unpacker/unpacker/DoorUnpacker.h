#ifndef DOORUNPACKER_H
#define DOORUNPACKER_H

#include "unpacker.h"
#include <string>

/**
 * @brief The DoorUnpacker class, used to unpack DOOR*.DAT files
 */

class DoorUnpacker : public Unpacker {
    public:
        DoorUnpacker(std::string filename);
        ~DoorUnpacker();

        int unpack();
        std::string getName();

};

#endif // DOORUNPACKER_H
