#include "PXLUnpacker.h"

PXLUnpacker::PXLUnpacker(std::string filename) : Unpacker (filename)
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
int PXLUnpacker::unpack() {
    this->convertToRGB888();
    this->saveAsPNG(this->filename+".png");

    return 1;
}

void PXLUnpacker::convertToRGB888() {
    this->rgb888Data = converter->convert(this->rgb555Data);
    this->isConverted = true;
    return;
}

bool PXLUnpacker::saveAsPNG(std::string filename) {
    if (!this->isConverted) {
        this->convertToRGB888();
    }

    auto image = new QImage(&rgb888Data[0], this->width, this->height, QImage::Format::Format_RGB888, nullptr, nullptr);
    return image->save(QString::fromStdString(this->filename+".png"), "PNG");
}