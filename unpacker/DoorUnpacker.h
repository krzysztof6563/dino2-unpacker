#ifndef DOORUNPACKER_H
#define DOORUNPACKER_H

#include "unpacker.h"
#include <string>
#include <map>
#include <QImage>
#include <cmath>

/**
 * @brief The DoorUnpacker class, used to unpack DOOR*.DAT files
 */

class DoorUnpacker : public Unpacker {
    public:
        DoorUnpacker(std::string filename);
        ~DoorUnpacker();

        int unpack();
        std::string getName();

        bool conv = 0;
        int bytesToSkipInPalette = 0;
        
    private:
        int mapBytesToChunks(int bytes);
        void dumpUnknownFile(int restSize);
        void dumpRemainingFile(int FILESIZE, int chunks);

        int DUMMY_HEADER_SIZE = 0xA00; //!< Size of dummy header at the beginnig of file
        int MODEL_SIZE; //!< Size of embeded 3d model
        int TEXTURE_SIZE; //!< Size of embeded texture

        char tmp[4]; //!< Holds "RIFF" value from wave file
        char oneByteBuffer;
        int width = 64;
        int height = 0;
};

#endif // DOORUNPACKER_H
