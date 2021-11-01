#include "StageDBSUnpacker.h"

StageDBSUnpacker::StageDBSUnpacker(std::string filename) : Unpacker (filename)
{
    BUFFER_SIZE = 64*32*4;
    PALETTE_SIZE = 64*8+64*24;
    CONVERTED_PALETTE_SIZE = PALETTE_SIZE/2*3;
    this->allocateVariables();
    p = new PaletteConverter(PALETTE_SIZE);
    
}

StageDBSUnpacker::~StageDBSUnpacker()
{
    delete p;
}

int StageDBSUnpacker::unpack()
{
    if (inFile.is_open()) {
        std::cout << "File" << filename << " opened." << std::endl;
        dechunker->dechunk();
        // dechunker->saveChunksToDisk();
        size_t total = dechunker->getNumberOfChunks();
        int imageNumber = 0;

        for (int i=0; i<dechunker->getNumberOfChunks(); i++) {
            char* chunk = dechunker->getChunkAt(i);
            // std::cout << "Chunk " << i << ": ";
            // for (size_t j = 0; j < dechunker->getChunkSize(); j++)
            // {
            //     printf("%02X ", chunk[j]);
            // }
            // std::cout << std::endl;

            if (this->isChunkJPEGStart(chunk)) {
                jpegStartPoints.push_back(i);
            }
        }

        if (jpegStartPoints.size() != 0) {
            if (!std::filesystem::is_directory(filename+"_jpgs")){
                std::filesystem::create_directory(filename+"_jpgs");
            }
            jpegStartPoints.push_back(dechunker->getNumberOfChunks());
        } else {
            std::cout << "[INFO] No JPEG files found" << std::endl;
        }

        for (size_t i = 0; i < jpegStartPoints.size() - 1; i++) {
            size_t startIndex = jpegStartPoints.at(i);
            std::cout << "[INFO] JPEG start found in chunk " << startIndex << std::endl;
            std::string outFilename = filename+"_jpgs" + "/" + std::to_string(i) + ".jpg";
            outFile.open(outFilename, std::fstream::binary);
            for (size_t j = startIndex; j < jpegStartPoints[i+1]; j++) {
                outFile.write(dechunker->getChunkAt(j), dechunker->getChunkSize());
            }
            outFile.close();
            std::cout << "[INFO] JPEG saved as " << outFilename << std::endl;
        }
        
        // if (!std::filesystem::is_directory("ITEM_u")){
        //     std::filesystem::create_directory("ITEM_u");
        // }
        
        inFile.close();
        std::cout << "[DEBUG] Closed file " << filename << std::endl;

    } else {
        std::cout << "Error opening  " << filename;
    }
    return 0;
}

bool StageDBSUnpacker::isChunkJPEGStart(char* chunk) {
    for (size_t i = 0; i < JPEG_START_LENGTH; i++) {
        if ((unsigned char)chunk[i] != JPEG_START[i]) {
            return false;
        }
    }

    return true;
}