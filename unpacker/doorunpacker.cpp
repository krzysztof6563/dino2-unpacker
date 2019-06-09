#include "doorunpacker.h"

DoorUnpacker::DoorUnpacker(std::string filename) : Unpacker (filename)
{
    BUFFER_SIZE = 64*32*4;
    PALETTE_SIZE = 64*8+64*24;
    CONVERTED_PALETTE_SIZE = PALETTE_SIZE/2*3;
    this->allocateVariables();
    p = new PaletteConverter(PALETTE_SIZE);
}

/**
 * @brief DoorUnpacker::unpack, extracts files from DOOR*.DAT file
 *
 * Possible files to extract:
 * - wave files (door sounds)
 * - texture of door
 * - 3d model of door (?)
 * @return
 */
int DoorUnpacker::unpack()
{
    if (inFile.is_open()) {
        std::cout << "File " << filename << " opened." << std::endl;
        inFile.close();
        inFile.open(filename, std::ifstream::ate | std::ifstream::binary);
        long int FILESIZE = inFile.tellg();
        inFile.seekg(0);
        inFile.ignore(DUMMY_HEADER_SIZE);
        while (inFile.read(tmp, 4)) {
            std::string b = tmp;
            if (b == "RIFF") {
                std::string outFilename = filename + "." + std::to_string(waveFiles) + ".WAV";
                std::cout << "Found WAVE file. Extracting to " << outFilename << std::endl;
                this->extractWAV(&inFile, outFilename, tmp);
                waveFiles++;
            } /*else if (b == "") {
                inFile.ignore(64*24-4);
            }*/ else {
                long int restSize = FILESIZE-inFile.tellg();
                char *rest = new char[restSize];
                std::cout << "Unkown format. Extracting remaining data (" + std::to_string(restSize) + " bytes) to " << filename+".data" << std::endl;
                outFile.open(filename+".data", std::fstream::binary);
                inFile.read(rest, restSize);
                outFile.write(rest, restSize);
                delete [] rest;
                outFile.close();
            }
        }
        inFile.close();
        dechunker->dechunk();
        dechunker->saveChunksToDisk();
    } else {
        std::cout << "Error opening  " << filename;
    }
    return 0;
}

/**
 * @brief DoorUnpacker::extractWAV, extract embeded wave file
 *
 * Wave structure (simplified):
 *
 * RIFF header + file size (little endian) + rest of file
 * @param inFile
 * @param outFilename
 * @param tmp RIFF header
 */
void DoorUnpacker::extractWAV(std::ifstream *inFile, std::string outFilename, char *tmp)
{
    char WAVE_SIZE[4];
    inFile->read(WAVE_SIZE, 4);
        WAVE_FILE_SIZE = 8+((unsigned char) WAVE_SIZE[3] << 24 |
                      (unsigned char) WAVE_SIZE[2] << 16 |
                      (unsigned char) WAVE_SIZE[1] << 8 |
                      (unsigned char) WAVE_SIZE[0] << 0 );
    std::cout << "Wave size: " << WAVE_FILE_SIZE << " bytes" << std::endl;
    WAVE_FILE = new char[WAVE_FILE_SIZE];
    for (int i = 0; i < 4; i++) {
        WAVE_FILE[i] = tmp[i];
        WAVE_FILE[i+4] = WAVE_SIZE[i];
    }
    inFile->read(WAVE_FILE+8, WAVE_FILE_SIZE-8); //substract 8 initial bytes used for header
    outFile.open(outFilename, std::fstream::binary);
    outFile.write(WAVE_FILE, WAVE_FILE_SIZE);
    outFile.close();
}

