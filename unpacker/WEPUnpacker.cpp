#include "WEPUnpacker.h"

WEPUnpacker::WEPUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->PNG_WIDTH = 64;
    this->PNG_HEIGHT = 64 ;
}

int WEPUnpacker::unpack() {
    this->findRIFFFiles();
    std::string outFilename = this->filename + ".data";
    this->outFile.open(outFilename);
    this->inFile.clear();
    unsigned int startPos = 0; 
    if (this->WAVE_FILES.size() > 0) {
        startPos = (unsigned int)this->WAVE_FILES.back()->getStart() + this->WAVE_FILES.back()->getSize(); 
        float currentChunk = ceil(startPos / (float)this->dechunker->getChunkSize());
        startPos = currentChunk * this->dechunker->getChunkSize();
    }
    unsigned int bytesToRead = this->inFileSize - startPos;
    this->inFile.seekg(startPos);
    char* buffer = new char[bytesToRead];
    this->inFile.read(buffer, bytesToRead);
    this->outFile.write(buffer, bytesToRead);

    std::cout << "[INFO] Saved as " << outFilename << "\n"; 

    return 0;
}

std::string WEPUnpacker::getName() {
    return "WEPUnpacker";
}