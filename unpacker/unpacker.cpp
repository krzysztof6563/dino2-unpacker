#include "unpacker.h"

Unpacker::Unpacker(std::string filename) {
    this->filename = filename;
    this->inFile.open(filename.c_str(), std::ios::binary);
    dechunker = new Dechunker(filename);
    this->converter = std::unique_ptr<RGBConverter>(new RGBConverter());
}

Unpacker::~Unpacker() {
    delete dechunker;
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
    std::cout << "Converting RGB555 to RGB888" << std::endl;
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
    auto image = new QImage(&this->rgb555Data[0] + offset, this->PNG_WIDTH, this->PNG_HEIGHT, QImage::Format::Format_Indexed8, nullptr, nullptr);

    this->colors.clear();
    for (size_t i = 0; i<paletteData.size(); i += 3) {
        this->colors.push_back((new QColor(paletteData[i], paletteData[i+1], paletteData[i+2]))->rgb());
    }
    image->setColorTable(this->colors);

    bool result = image->save(QString::fromStdString(fileNameBase+".png"), "PNG");
    if (result) {
        std::cout << "[INFO] Saved image as " << fileNameBase << ".png" << std::endl;
    } else {
        std::cout << "[ERRO] There was an error while saving " << fileNameBase << ".png" << std::endl;
    }

    return result;
}

/**
 * Saves coverted data to RGB888 PNG image 
 * 
 * @param std::string outFileName
*/
bool Unpacker::saveAsRGB888PNG(std::string fileNameBase, int offset) {
    auto image = new QImage(&rgb888Data[0] + offset, this->PNG_WIDTH, this->PNG_HEIGHT, QImage::Format::Format_RGB888, nullptr, nullptr);
    bool result = image->save(QString::fromStdString(fileNameBase+".png"), "PNG");
    if (result) {
        std::cout << "[INFO] Saved image as " << fileNameBase << ".png" << std::endl;
    } else {
        std::cout << "[ERRO] There was an error while saving " << fileNameBase << ".png" << std::endl;
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
