#include "doorunpacker.h"

DoorUnpacker::DoorUnpacker(std::string filename) : Unpacker (filename) {
    BUFFER_SIZE = 64*32*4;
    PALETTE_SIZE = 64*8+64*24;
    CONVERTED_PALETTE_SIZE = PALETTE_SIZE/2*3;
    this->allocateVariables();
    p = new PaletteConverter(PALETTE_SIZE);
}

DoorUnpacker::~DoorUnpacker() {
    for (auto p : WAVE_FILES) {
        delete p;
    }
    WAVE_FILES.clear();
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
int DoorUnpacker::unpack() {
    if (inFile.is_open()) {
        unsigned long long int restSize = 0;
        std::cout << "File " << filename << " opened." << std::endl;
        inFile.close();
        inFile.open(filename, std::ifstream::ate | std::ifstream::binary);
        long int FILESIZE = inFile.tellg();
        inFile.seekg(0);
        inFile.ignore(DUMMY_HEADER_SIZE);
        //FIND ALL RIFF files in file
        while (inFile.read(tmp, 4)) {
            std::string b = tmp;
            if (b == "RIFF") {
                std::streampos start = inFile.tellg();
                start -= 4;
                WAVE_FILES.push_back(new WaveFile(start));
                std::cout << "Found WAVE start at " << start << " adding to vector." << std::endl ;
            } /*else if (b == "") {
                inFile.ignore(64*24-4);
            } else {
                 find first non zero byte
                 * Next starting bytes: 02 07 00 05 00 00 00 00 02 07 00 15 00 00 00 00 02 07 00 0A 13
                 *
                long int restSize = FILESIZE-inFile.tellg();
                int headerSize = 21;
                for (int i=0;i<restSize;i++) {
                    inFile.read(&oneByteBuffer, 1);
                    if (oneByteBuffer == 0x02) {
                        char *headerBuffer = new char[headerSize];
                        std::cout << "Found non zero byte at " << inFile.tellg() << std::endl;
                        int currentPos = inFile.tellg();
                        inFile.seekg(currentPos-headerSize);
                        inFile.read(headerBuffer, headerSize);
                        std::string hB(headerBuffer);
                        if (hB == "020700050000000002070015000000000207000A13") {
                            std::cout << "Found magic string" << std::endl;
                        }
                        delete[] headerBuffer;
                        break;
                    } else {
                        continue;
                    }  
                }

                restSize = FILESIZE-inFile.tellg()+1;
                int currentPos = inFile.tellg();
                inFile.seekg(currentPos-1);
                char *rest = new char[restSize];
                std::cout << "Unkown format. Extracting remaining data (" + std::to_string(restSize) + " bytes) to " << filename+".data" << std::endl;
                outFile.open(filename+".data", std::fstream::binary);
                inFile.read(rest, restSize);
                outFile.write(rest, restSize);
                delete [] rest;
                outFile.close();

            } */

        }
        //go back to beginning of file
        inFile.clear();
        inFile.seekg(0);
        //extract all found RIFF files
        for (WaveFile* waveFile : WAVE_FILES) {
            inFile.seekg(waveFile->getStart());
            std::string outFilename = filename + "." + std::to_string(waveFiles) + ".WAV";
            std::cout << "Extracting WAVE file to " << outFilename << std::endl;
            this->extractWAV(&inFile, outFilename, waveFile);
            waveFiles++;
        }
        //seekg to last byte of last wave file 
        inFile.seekg(WAVE_FILES.back()->getSize()+WAVE_FILES.back()->getStart());
        while (inFile.read(tmp, 1)) {
            if (tmp[0] != 0x00) {
                unsigned long long int restSize = FILESIZE-inFile.tellg()+1;
                int currentPos = inFile.tellg();
                inFile.seekg(currentPos-1);
                char *rest = new char[restSize];
                std::cout << "Unkown format. Extracting remaining data (" + std::to_string(restSize) + " bytes) to " << filename+".data" << std::endl;
                outFile.open(filename+".data", std::fstream::binary);
                inFile.read(rest, restSize);
                outFile.write(rest, restSize);
                delete [] rest;
                outFile.close();
                break;
            }
        }
        inFile.close();
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
 */
void DoorUnpacker::extractWAV(std::ifstream *inFile, std::string outFilename, WaveFile* waveFile) {
    const char *RIFF_HEADER = "RIFF";
    inFile->ignore(4);
    char WAVE_SIZE[4];
    inFile->read(WAVE_SIZE, 4);
    WAVE_FILE_SIZE = 8+((unsigned char) WAVE_SIZE[3] << 24 |
                      (unsigned char) WAVE_SIZE[2] << 16 |
                      (unsigned char) WAVE_SIZE[1] << 8 |
                      (unsigned char) WAVE_SIZE[0] << 0 );
    waveFile->setSize(WAVE_FILE_SIZE);
    std::cout << "Wave size: " << WAVE_FILE_SIZE << " bytes" << std::endl;
    WAVE_FILE = new char[WAVE_FILE_SIZE];
    for (int i = 0; i < 4; i++) {
        WAVE_FILE[i] = RIFF_HEADER[i];
        WAVE_FILE[i+4] = WAVE_SIZE[i];
    }
    inFile->read(WAVE_FILE+8, WAVE_FILE_SIZE-8); //substract 12 initial bytes used for header
    outFile.open(outFilename, std::fstream::binary);
    //add RIFF to beginning
    outFile.write(WAVE_FILE, WAVE_FILE_SIZE);
    outFile.close();
}

