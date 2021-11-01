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
        unsigned long long int FILESIZE = inFile.tellg();
        inFile.seekg(0);
        inFile.ignore(DUMMY_HEADER_SIZE);
        //FIND ALL RIFF files in file
        while (inFile.read(tmp, 4)) {
            std::string b = tmp;
            if (b == "RIFF") {
                //go back 4 bytes to compensate for reading RIFF
                std::streampos start = inFile.tellg();
                start -= 4;
                //add new WaveFile to our vector
                WAVE_FILES.push_back(new WaveFile(start));
                std::cout << "Found WAVE start at " << start << " adding to vector." << std::endl ;
            }
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
        /* seekg to last byte of last wave file, and find next byte that isn't 0x00
         * It marks the start of bitmap for current door   
         * It's usually this:  
         * 02 07 00 05 00 00 00 00 02 07 00 15 00 00 00 00 02 07 00 0A 13  
         * Read chunks, check if starts with 0x00? 
        */
        inFile.seekg(WAVE_FILES.back()->getSize()+WAVE_FILES.back()->getStart());
        while (inFile.read(tmp, 1)) {
            if (tmp[0] != 0x00) {
                unsigned long long int restSize = FILESIZE-inFile.tellg()+1;
                int currentPos = inFile.tellg();
                inFile.seekg(currentPos-1);
                std::string rs = std::to_string(restSize);
                int chunks = mapBytesToChunks(restSize);
                std::cout << "Remaining bytes ----" << restSize << "-----" << std::endl;
                if (chunks == 0) {
                    //we don't know the data beyond this point, so we are just dumping it 
                    this->dumpUnknownFile(restSize);
                    break;
                } else {
                    this->dumpRemainingFile(FILESIZE, chunks);
                    break;
                }
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
 * RIFF header (4 bytes) + file size (little endian) (4 bytes) + rest of file
 * @param inFile
 * @param outFilename
 */
void DoorUnpacker::extractWAV(std::ifstream *inFile, std::string outFilename, WaveFile* waveFile) {
    inFile->ignore(4);
    char WAVE_SIZE[4];
    inFile->read(WAVE_SIZE, 4);
    // convert size header so we can use it
    WAVE_FILE_SIZE = 8+((unsigned char) WAVE_SIZE[3] << 24 |
                      (unsigned char) WAVE_SIZE[2] << 16 |
                      (unsigned char) WAVE_SIZE[1] << 8 |
                      (unsigned char) WAVE_SIZE[0] << 0 );
    waveFile->setSize(WAVE_FILE_SIZE);
    std::cout << "Wave size: " << WAVE_FILE_SIZE << " bytes" << std::endl;
    // and now to make a new file in a memory (header and size info)
    WAVE_FILE = new char[WAVE_FILE_SIZE];
    for (int i = 0; i < 4; i++) {
        WAVE_FILE[i] = this->RIFF_HEADER[i];
        WAVE_FILE[i+4] = WAVE_SIZE[i];
    }
    // start at 8th byte in new wave file (size of header and size info)
    // in consequence we have to read 8 bytes less than actual size 
    inFile->read(WAVE_FILE+8, WAVE_FILE_SIZE-8); 
    outFile.open(outFilename, std::fstream::binary);
    outFile.write(WAVE_FILE, WAVE_FILE_SIZE);
    outFile.close();
    delete[] WAVE_FILE;
}

int DoorUnpacker::mapBytesToChunks(int bytes) {
    /* TODO
    These chunks numbers gives weird palette - colors are off
    16
    8
    5 
    4
    */
    switch (bytes) {
        case 49152:
        case 38912:
            this->conv = true;
            return 17;

        case 45056:
        case 43008:
        case 40960:
            this->conv = false;
            this->bytesToSkipInPalette = 15;
            return 16;

        case 20480:
            //skip 15 bytes
            this->conv = false;
            this->bytesToSkipInPalette = 15;
            return 4;

        case 16384:
            //skip 15 bytes
            this->conv = false;
            this->bytesToSkipInPalette = 15;
            return 5;

        case 22528:
            //idk what happens here
            this->conv = false;
            this->bytesToSkipInPalette = 15;
            return 8;

        default:
            return 0;
    }
}

void DoorUnpacker::dumpUnknownFile(int restSize) {
    char *rest = new char[restSize];
    std::cout << "Unkown format. Extracting remaining data (" + std::to_string(restSize) + " bytes) to " << filename+".data" << std::endl;
    outFile.open(filename+".data", std::fstream::binary);
    inFile.read(rest, restSize);
    outFile.write(rest, restSize);
    delete [] rest;
    outFile.close();
}

void DoorUnpacker::dumpRemainingFile(int FILESIZE, int chunks) {
    std::cout << "Dumping image, palette and model data to respective files." << std::endl;
    //we do know how many chunks we need to extract
    int sizeOfChunks = chunks*dechunker->getChunkSize();
    int modelSize = FILESIZE - sizeOfChunks - dechunker->getChunkSize();
    // inFile.ignore(this->dechunker->getChunkSize());

    char *image = new char[sizeOfChunks];
    char *palette = new char[dechunker->getChunkSize()]; 
    char *model = new char[modelSize]; 

    inFile.read(image, sizeOfChunks);
    inFile.read(palette, dechunker->getChunkSize());
    inFile.read(model, modelSize);

    outFile.open(filename+".data", std::fstream::binary);
    outFile.write(image, sizeOfChunks);
    outFile.close();

    if (this->conv) {
        p->setPalette(palette);
        p->setNewPalette(newPalette);
        p->convert();
    }
    
    outFile.open(filename+".data.pal", std::fstream::binary);
    outFile.write(newPalette+this->bytesToSkipInPalette, CONVERTED_PALETTE_SIZE-this->bytesToSkipInPalette);

    outFile.close();

    outFile.open(filename+".data.model", std::fstream::binary);
    outFile.write(model, modelSize);
    outFile.close();

    delete[] model;
    delete[] palette;
    delete[] image;
}