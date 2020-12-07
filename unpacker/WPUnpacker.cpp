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

//save new image to array
//save image to PNG
//separate rgb555 array with rgb888
//keep track if image was converted
int WPUnpacker::unpack() {
    auto palette = this->dechunker->getChunkAt(this->PALLETTE_CHUNK);
    for(size_t j = 0; j<this->dechunker->getChunkSize(); j++) {
        paletteData.push_back(palette[j]);
    }
    paletteData = this->converter->convert(paletteData);

    this->saveAsPNG(this->filename+".png");

    return 1;
}

void WPUnpacker::convertToRGB888() {
    this->rgb888Data = converter->convert(this->rgb555Data);
    this->isConverted = true;
    return;
}

bool WPUnpacker::saveAsPNG(std::string filename) {
    if (!this->isConverted) {
        this->convertToRGB888();
    }

    int imageHeight = (this->dechunker->getNumberOfChunks()-2)*32;

    auto image = new QImage(&rgb555Data[this->dechunker->getChunkSize()], this->width, imageHeight, QImage::Format::Format_Indexed8, nullptr, nullptr);
    for (size_t i = 0; i<paletteData.size(); i += 3) {
        this->colors.push_back((new QColor(paletteData[i], paletteData[i+1], paletteData[i+2]))->rgb());
    }

    image->setColorTable(this->colors);
    return image->save(QString::fromStdString(this->filename+".png"), "PNG");
}