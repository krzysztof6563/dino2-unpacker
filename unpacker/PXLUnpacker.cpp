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
    this->saveAsPNG(this->filename);
    std::cout << "Saved as PNG file named: " << this->filename << ".png" << std::endl; 

    return 1;
}

bool PXLUnpacker::saveAsPNG(std::string outFileName) {
    if (!this->isConverted) {
        this->convertToRGB888();
    }

    auto image = new QImage(&rgb888Data[0], this->width, this->height, QImage::Format::Format_RGB888, nullptr, nullptr);
    return image->save(QString::fromStdString(outFileName+".png"), "PNG");
}