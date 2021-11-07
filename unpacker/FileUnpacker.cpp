#include "FileUnpacker.h"

FileUnpacker::FileUnpacker(std::string filename) : Unpacker (filename) {
    this->dechunker->dechunk();
    this->converter = std::unique_ptr<RGBConverter>(new RGBConverter());
}

int FileUnpacker::unpack() {
    int numberOfchunks = this->dechunker->getNumberOfChunks();
    int numberOfImages = numberOfchunks / 7;

    for (size_t i = 0; i < numberOfImages; i++)
    {
        this->rgb555Data.clear();
        this->paletteData.clear();
        for (size_t j = 0; j < 6; j++)
        {
            char* chunk = dechunker->getChunkAt(i*7 + j);
            for (size_t k = 0; k < this->dechunker->getChunkSize(); k++) {
                this->rgb555Data.push_back(chunk[k]);
            }
            
        }

        char* palletteChunk = this->dechunker->getChunkAt(i*7 + 6);
        for (size_t k = 0; k < this->dechunker->getChunkSize(); k++) {
            this->paletteData.push_back(palletteChunk[k]);
        }
        paletteData = this->converter->convert(paletteData);

        this->saveAsPNG(this->filename + "_" + std::to_string(i));
    }
    
    // }
    return 1;
}

/**
 * Saves coverted data to PNG image 
 * 
 * @param std::string outFileName
*/
bool FileUnpacker::saveAsPNG(std::string outFileName) {
    int headerOffset = this->dechunker->getChunkSize();

    auto image = new QImage(&this->rgb555Data[0], 64, 32 * 6, QImage::Format::Format_Indexed8, nullptr, nullptr);
    for (size_t i = 0; i<paletteData.size(); i += 3) {
        this->colors.push_back((new QColor(paletteData[i], paletteData[i+1], paletteData[i+2]))->rgb());
    }

    image->setColorTable(this->colors);
    std::cout << "Saving image data to "+outFileName+".png" << std::endl;
    return image->save(QString::fromStdString(outFileName+".png"), "PNG");
}

std::string FileUnpacker::getName() {
    return "FileUnpacker";
}