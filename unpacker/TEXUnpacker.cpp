#include "TEXUnpacker.h"

TEXUnpacker::TEXUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->loadChunksToRGB555Vector();
}

int TEXUnpacker::unpack() {
    int numberOfchunks = this->dechunker->getNumberOfChunks()-1;
    int imageHeight = numberOfchunks == 9 ? 8*32 : (34-2)*32;
    int paletteIndex = numberOfchunks == 9 ? 9 : 34-1;
    auto palette = this->dechunker->getChunkAt(paletteIndex); //34-1 is index of first palette
    
    for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
        paletteData.push_back(palette[j]);
    }
    paletteData = this->converter->convert(paletteData);

    this->PNG_HEIGHT = imageHeight;
    this->saveAsIndexedPNG(this->filename, this->dechunker->getChunkSize());

    if (numberOfchunks == 66) {
        paletteData.clear();
        palette = this->dechunker->getChunkAt(numberOfchunks);
        for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
            paletteData.push_back(palette[j]);
        }
        paletteData = this->converter->convert(paletteData);

        this->saveAsIndexedPNG(this->filename+"_2", (34)*this->dechunker->getChunkSize());
    }
    return 1;
}

/**
 * Saves coverted data to PNG image 
 * 
 * @param std::string outFileName
 * @param int offset Offset to start image from  
 * @param int imageHeight Height of image to be saved  
*/
bool TEXUnpacker::saveAsPNG(std::string outFileName, int offset, int imageHeight) {
    int headerOffset = this->dechunker->getChunkSize();

    auto image = new QImage(&rgb555Data[headerOffset+offset], this->width, imageHeight, QImage::Format::Format_Indexed8, nullptr, nullptr);
    for (size_t i = 0; i<paletteData.size(); i += 3) {
        this->colors.push_back((new QColor(paletteData[i], paletteData[i+1], paletteData[i+2]))->rgb());
    }

    image->setColorTable(this->colors);
    std::cout << "Saving image data to "+outFileName+".png" << std::endl;
    return image->save(QString::fromStdString(outFileName+".png"), "PNG");
}

std::string TEXUnpacker::getName() {
    return "TEXUnpacker";
}