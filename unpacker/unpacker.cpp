#include "unpacker.h"

Unpacker::Unpacker(std::string filename) {
    this->filename = filename;
    this->inFile.open(filename.c_str(), std::ios::binary | std::ios::ate);
    if (this->inFile.is_open()) {
        this->inFileSize = this->inFile.tellg();
    }
    this->inFile.seekg(0);
    std::cout << "[DEBUG] File " << filename << " opened." << '\n';
    dechunker = new Dechunker(filename);
    this->converter = std::unique_ptr<RGBConverter>(new RGBConverter());
}

Unpacker::~Unpacker() {
    delete dechunker;

    for (auto waveFile : WAVE_FILES) {
        delete waveFile;
    }
    WAVE_FILES.clear();
}

char *Unpacker::getBuffer() const {
    return buffer;
}

void Unpacker::setBuffer(char *value) {
    buffer = value;
}

char *Unpacker::getPalette() const {
    return palette;
}

char *Unpacker::getNewPalette() const {
    return newPalette;
}

void Unpacker::setNewPalette(char *value) {
    newPalette = value;
}

void Unpacker::convertToRGB888() {
    std::cout << "Converting RGB555 to RGB888" << '\n';
    this->rgb888Data = converter->convert(this->rgb555Data);
    this->isConverted = true;
    return;
}

bool Unpacker::saveChunks() {
    this->dechunker->saveChunksToDisk();

    return true;
}   

/**
 * Saves coverted data to indexed PNG image 
 * 
 * @param std::string fileNameBase Base of filename to save as
 * @param std::string offset Offset from beginning of rgb555Data vector
*/
bool Unpacker::saveAsIndexedPNG(std::string fileNameBase, int offset) {
    auto image = new QImage(this->rgb555Data.data() + offset, this->PNG_WIDTH, this->PNG_HEIGHT, QImage::Format::Format_Indexed8, nullptr, nullptr);

    this->colors.clear();
    for (size_t i = 0; i<paletteData.size(); i += 3) {
        this->colors.push_back((new QColor(paletteData[i], paletteData[i+1], paletteData[i+2]))->rgb());
    }
    image->setColorTable(this->colors);

    bool result = image->save(QString::fromStdString(fileNameBase+".png"), "PNG");
    if (result) {
        std::cout << "[INFO] Saved image as " << fileNameBase << ".png" << '\n';
    } else {
        std::cout << "[ERRO] There was an error while saving " << fileNameBase << ".png" << '\n';
    }

    return result;
}

/**
 * Extracts images embeded in file
 * 
 * @param int How many images (sequential) to extract
 * @param int How tall (in chunks) the image is
 * @param int How many chunks from start of file to skip 
 * @param int Passed to rearrange function
 * @param int Passed to rearrange function
 * @param int Passed to rearrange function
 *  
 */
int Unpacker::extractImages(
    unsigned int numberOfImages, 
    unsigned int chunkHeight, 
    unsigned int initialOffset,
    unsigned int rearrangeWidth,
    unsigned int rearrangeHeight,
    unsigned int rearrangeStart
) {
    int numberOfchunks = this->dechunker->getNumberOfChunks();
    int chunkSkip = chunkHeight + 1;
    bool isSingleImage = numberOfImages == 1;

    if (!isSingleImage && !std::filesystem::is_directory(this->filename+"_pngs")){
        std::filesystem::create_directory(this->filename+"_pngs");
    }

    for (size_t i = 0; i < numberOfImages; i++)
    {
        this->rgb555Data.clear();
        this->paletteData.clear();
        for (size_t j = 0; j < chunkHeight; j++) {
            char* chunk = dechunker->getChunkAt(i*chunkSkip + j + initialOffset);
            for (size_t k = 0; k < this->dechunker->getChunkSize(); k++) {
                this->rgb555Data.push_back(chunk[k]);
            }
        }

        if (rearrangeWidth && rearrangeHeight) {
            this->rgb555Data = this->rearrangeChunks(rearrangeWidth, rearrangeHeight, rearrangeStart, this->rgb555Data);    
        }

        char* palletteChunk = this->dechunker->getChunkAt(i*chunkSkip + chunkHeight + initialOffset);
        for (size_t k = 0; k < this->dechunker->getChunkSize(); k++) {
            this->paletteData.push_back(palletteChunk[k]);
        }
        paletteData = this->converter->convert(paletteData);

        std::string outFileName = isSingleImage ? this->filename : this->filename + "_pngs/" + std::to_string(i);
        this->saveAsIndexedPNG(outFileName);
    }

    return 0;
}

/**
 * Saves coverted data to RGB888 PNG image 
 * 
 * @param std::string outFileName
*/
bool Unpacker::saveAsRGB888PNG(std::string fileNameBase, int offset) {
    auto image = new QImage(this->rgb888Data.data() + offset, this->PNG_WIDTH, this->PNG_HEIGHT, QImage::Format::Format_RGB888, nullptr, nullptr);
    bool result = image->save(QString::fromStdString(fileNameBase+".png"), "PNG");
    if (result) {
        std::cout << "[INFO] Saved image as " << fileNameBase << ".png" << '\n';
    } else {
        std::cout << "[ERRO] There was an error while saving " << fileNameBase << ".png" << '\n';
    }

    return result;
}

void Unpacker::loadChunksToRGB555Vector() {
    auto size = this->dechunker->getNumberOfChunks();
    //load chunks data to rgb555 array;
    for (size_t i = 0; i < size; i++) {
        auto chunk = this->dechunker->getChunkAt(i);
        for (int j = 0; j < dechunker->getChunkSize(); j++) {
            this->rgb555Data.push_back(chunk[j]);
        }
    }
}

std::vector<uint8_t> Unpacker::rearrangeChunks(int width, int height, int start, std::vector<uint8_t> initialData) {
    int chunkSize = this->dechunker->getChunkSize();
    int stop = width * height + start;
    std::vector<uint8_t> newData;

    for (size_t i = start; i < stop; i += width) { //rows of chunks for whole image
        for (size_t j = 0; j < 32; j++) // rows for chunk height
        {
            for (size_t k = 0; k < width; k++) //loop over chunks in a row
            {
                int chunkStart = (i + k) * chunkSize;
                int offset = (64 * j);
                std::copy(
                    initialData.begin() + chunkStart + offset, 
                    initialData.begin() + chunkStart + offset + 64, 
                    std::back_inserter(newData)
                );
            }
        }
    }

    return newData;
}

int Unpacker::findAndExtractRIFFFiles() {
    this->findRIFFFiles();
    this->extractAllRIFFFIles();

    return this->WAVE_FILES.size();
}

void Unpacker::findRIFFFiles() {
    char localBuffer[5] = {'\0'};
    if (this->inFile.is_open()) {
        while (inFile.read(localBuffer, 1))
        {
            if (localBuffer[0] == 'R') {
                // inFile.seekg((unsigned int)inFile.tellg() - 1);
                inFile.read(localBuffer + 1, 3);

                if (strncmp(localBuffer, "RIFF", 4) == 0) {
                    unsigned int startPos = (unsigned int)inFile.tellg() - 4;
                    std::cout << "[DEBUG] Found RIFF header at 0x" << std::hex << startPos << "\n";
                    this->WAVE_FILES.push_back(new WaveFile(startPos));
                }
            }
        }
        
    }
}

void Unpacker::extractAllRIFFFIles() {
    int waveFiles = 0;
     //extract all found RIFF files
    for (WaveFile* waveFile : this->WAVE_FILES) {
        std::string outFilename = this->filename + "." + std::to_string(waveFiles) + ".WAV";
        std::cout << "[INFO] Extracting WAVE file to " << outFilename << '\n';
        this->extractRIFFFile(outFilename, waveFile);
        waveFiles++;
    }
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
void Unpacker::extractRIFFFile(std::string outFilename, WaveFile* waveFile) {
    this->inFile.clear();
    this->inFile.seekg(waveFile->getStart(), std::ios::beg);
    this->inFile.ignore(4);
    char WAVE_SIZE[5] = {'\0'};
    this->inFile.read(WAVE_SIZE, 4);
    // convert size header so we can use it
    uint32_t WAVE_FILE_SIZE = 8+((unsigned char) WAVE_SIZE[3] << 24 |
                      (unsigned char) WAVE_SIZE[2] << 16 |
                      (unsigned char) WAVE_SIZE[1] << 8 |
                      (unsigned char) WAVE_SIZE[0] << 0 );
    waveFile->setSize(WAVE_FILE_SIZE);
    std::cout << "[DEBUG] Wave size: 0x" << WAVE_FILE_SIZE << " bytes" << '\n';
    // and now to make a new file in a memory (header and size info)
    char* WAVE_FILE = new char[WAVE_FILE_SIZE];
    for (int i = 0; i < 4; i++) {
        WAVE_FILE[i] = this->RIFF_HEADER[i];
        WAVE_FILE[i+4] = WAVE_SIZE[i];
    }
    // start at 8th byte in new wave file (size of header and size info)
    // in consequence we have to read 8 bytes less than actual size 
    this->inFile.read(WAVE_FILE+8, WAVE_FILE_SIZE-8); 
    outFile.open(outFilename, std::fstream::binary);
    outFile.write(WAVE_FILE, WAVE_FILE_SIZE);
    outFile.close();
    delete[] WAVE_FILE;
}