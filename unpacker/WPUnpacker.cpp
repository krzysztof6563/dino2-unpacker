#include "WPUnpacker.h"

WPUnpacker::WPUnpacker(std::string filename) : Unpacker (filename)
{
    this->dechunker->dechunk();
    auto size = this->dechunker->getNumberOfChunks();
    //load chunks data to rgb555 array;
    for (size_t i=0; i<this->dechunker->getNumberOfChunks(); i++) {
        auto chunk = this->dechunker->getChunkAt(i);
        for (int j=0; j<dechunker->getChunkSize(); j++) {
            this->rgb555Data.push_back(chunk[j]);
        }
    }
    this->converter = std::unique_ptr<RGBConverter>(new RGBConverter());
}

int WPUnpacker::unpack() {
    auto palette = this->dechunker->getChunkAt(this->PALLETTE_CHUNK);
    for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
        paletteData.push_back(palette[j]);
    }
    paletteData = this->converter->convert(paletteData);

    this->saveAsPNG(this->filename);

    return 1;
}

bool WPUnpacker::saveAsPNG(std::string outFileName) {
    int imageHeight = (this->dechunker->getNumberOfChunks()-2)*32;
    int headerOffset = this->dechunker->getChunkSize();

    auto image = new QImage(&rgb555Data[headerOffset], this->width, imageHeight, QImage::Format::Format_Indexed8, nullptr, nullptr);
    for (size_t i = 0; i<paletteData.size(); i += 3) {
        this->colors.push_back((new QColor(paletteData[i], paletteData[i+1], paletteData[i+2]))->rgb());
    }

    image->setColorTable(this->colors);
    return image->save(QString::fromStdString(outFileName+".png"), "PNG");
}