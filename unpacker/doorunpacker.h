#ifndef DOORUNPACKER_H
#define DOORUNPACKER_H

#include "unpacker.h"
#include "../converter/paletteconverter.h"
#include <string>

/**
 * @brief The DoorUnpacker class, used to unpack DOOR*.DAT files
 */

class DoorUnpacker : public Unpacker
{
public:
    DoorUnpacker(std::string filename);

    int unpack();
    void extractWAV(std::ifstream *inFile, std::string outFilename, char *tmp);
private:
    int DUMMY_HEADER_SIZE = 0xA00; //!< Size of dummy header at the beginnig of file
    uint32_t WAVE_FILE_SIZE; //!< Size of embeded wave file
    int MODEL_SIZE; //!< Size of embeded 3d model
    int TEXTURE_SIZE; //!< Size of embeded texture
    int waveFiles = 0; //!< Number of embeded wave files

    char *WAVE_FILE; //!< Pointer to array containing wave file data
    char tmp[4]; //!< Holds "RIFF" value from wave file
    PaletteConverter *p;
};

#endif // DOORUNPACKER_H
