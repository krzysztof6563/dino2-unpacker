#include "DoorUnpacker.h"

DoorUnpacker::DoorUnpacker(std::string filename) : Unpacker (filename) {}

DoorUnpacker::~DoorUnpacker() {
    for (auto waveFile : WAVE_FILES) {
        delete waveFile;
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
        std::cout << "[DEBUG] File " << filename << " opened." << std::endl;
        inFile.close();
        inFile.open(filename, std::ifstream::ate | std::ifstream::binary);
        unsigned long long int FILESIZE = inFile.tellg();

        /* seek to nearest full chunk */
        int currentPosition = WAVE_FILES.back()->getSize() + WAVE_FILES.back()->getStart(); 
        float currentChunk = ceil(currentPosition / (float)this->dechunker->getChunkSize());
        inFile.seekg(currentChunk * this->dechunker->getChunkSize());;
        
        std::cout << "[DEBUG] Curernt position: " << inFile.tellg() << ". Chunk: " << currentChunk << std::endl;
        restSize = FILESIZE-inFile.tellg();
        int currentPos = inFile.tellg();
        std::string rs = std::to_string(restSize);
        int chunks = mapBytesToChunks(restSize);
        std::cout << "[DEBUG] Remaining bytes ----" << restSize << "-----" << std::endl;
        if (chunks == 0) {
            //we don't know the data beyond this point, so we are just dumping it 
            this->dumpUnknownFile(restSize);
        } else {
            this->dumpRemainingFile(restSize, chunks);
        }

        inFile.close();
    } else {
        std::cout << "[ERROR] Error opening  " << filename;
    }
    
    return 0;
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
        case 45056:
        case 43008:
        case 40960:
            return 17;

        case 20480:
            return 4;

        case 16384:
            return 5;

        case 22528:
            //idk what happens here
            return 8;

        default:
            return 0;
    }
}

void DoorUnpacker::dumpUnknownFile(int restSize) {
    char *rest = new char[restSize];
    std::cout << "[INFO] Unkown format. Extracting remaining data (" + std::to_string(restSize) + " bytes) to " << filename+".data" << std::endl;
    outFile.open(filename+".data", std::fstream::binary);
    inFile.read(rest, restSize);
    outFile.write(rest, restSize);
    delete [] rest;
    outFile.close();
}

/**
 * Save image, palette, 3D data to files
 *
 * @param int restSize Bytes remaining in file (no header and WAVE files) 
 * @param chunks Number of chunks that image is embedded in
 */
void DoorUnpacker::dumpRemainingFile(int restSize, int chunks) {
    std::cout << "[INFO] Saving image, palette and model data to respective files." << std::endl;
    int sizeOfChunks = chunks * dechunker->getChunkSize(); //!< Size of image
    int modelSize = restSize - sizeOfChunks - dechunker->getChunkSize(); //!< Size of 3D data (LEFT - IMAGE - palette)
    // inFile.ignore(this->dechunker->getChunkSize());

    char *image = new char[sizeOfChunks];
    char *palette = new char[dechunker->getChunkSize()]; 
    char *model = new char[modelSize]; 

    inFile.read(image, sizeOfChunks);
    inFile.read(palette, dechunker->getChunkSize());
    // inFile.ignore(this->dechunker->getChunkSize());
    // inFile.read(model, modelSize);
    uint16_t vertexOffset, pad, normalOffset, TriangleOffset, QuadOffset, VertexCount, ObjectCount;
    uint16_t data[10 + 12];
    // inFile.read((char*)&vertexOffset, sizeof(vertexOffset));
    // inFile.read((char*)&pad, sizeof(pad));
    // inFile.read((char*)&normalOffset, sizeof(normalOffset));
    // inFile.read((char*)&pad, sizeof(pad));
    // inFile.read((char*)&TriangleOffset, sizeof(TriangleOffset));
    // inFile.read((char*)&pad, sizeof(pad));
    // // inFile.read((char*)&QuadOffset, sizeof(QuadOffset));
    // // inFile.read((char*)&pad, sizeof(pad));
    // inFile.read((char*)&VertexCount, sizeof(VertexCount));
    // inFile.read((char*)&pad, sizeof(pad));
    // inFile.read((char*)&ObjectCount, sizeof(ObjectCount));
    // inFile.read((char*)&pad, sizeof(pad));

    // // inFile.ignore(24);
    // inFile.ignore(2*4);
    // uint16_t triangleOffset, quadOffset, triangleCount, quadCount;
    // inFile.read((char*)&triangleOffset, sizeof(triangleOffset));
    // inFile.ignore(2);
    // inFile.read((char*)&quadOffset, sizeof(quadOffset));
    // inFile.ignore(2);
    // inFile.read((char*)&triangleCount, sizeof(triangleCount));
    // inFile.read((char*)&quadCount, sizeof(quadCount));
    for (size_t i = 0; i < 10+12; i++)
    {
        inFile.read((char*)&data[i], 2);
    }
    
    int j = 0;
    std::cout << "MAIN HEADER" << "\n";
    
    for (auto i : data) {
        std::cout << std::dec << i << "\t";
        if (j == 23) {
            std::cout << "\n\n" << "OBJECT HEADER" << "\n";
        }
        j++;
    }
    std::cout << "\n\n";

    outFile.open(filename+".data", std::fstream::binary);
    outFile.write(image, sizeOfChunks);
    outFile.close();

    for (size_t i = 0; i < this->dechunker->getChunkSize(); i++) {
        this->paletteData.push_back(palette[i]);
    }
    this->paletteData = this->converter->convert(paletteData);

    for (size_t i = 0; i < sizeOfChunks; i++) {
        this->rgb555Data.push_back(image[i]);
    }

    
    outFile.open(filename+".data.pal", std::fstream::binary);
    for (auto c : paletteData) outFile << c;
    outFile.close();

    outFile.open(filename+".data.model", std::fstream::binary);
    outFile.write(model, modelSize);
    outFile.close();

    this->PNG_HEIGHT = chunks * 32;
    this->saveAsIndexedPNG(filename);

    delete[] model;
    delete[] palette;
    delete[] image;
}

std::string DoorUnpacker::getName() {
    return "DoorUnpacker";
}