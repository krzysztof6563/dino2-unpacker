#ifndef DOORUNPACKER_H
#define DOORUNPACKER_H

#include "unpacker.h"
#include "../converter/paletteconverter.h"
#include <string>

class DoorUnpacker : public Unpacker
{
public:
    DoorUnpacker(std::string filename);

    int unpack();
    void extractWAV(std::ifstream *inFile, std::string outFilename, char *tmp);
private:
    int DUMMY_HEADER_SIZE = 0xA00;
    uint32_t WAVE_FILE_SIZE;
    int MODEL_SIZE;
    int TEXTURE_SIZE;
    int waveFiles = 0;

    char *WAVE_FILE;
    char tmp[4];
    PaletteConverter *p;
};

#endif // DOORUNPACKER_H
