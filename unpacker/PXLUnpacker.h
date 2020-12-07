#ifndef PXLUNPACKER_H
#define PXLUNPACKER_H

#include "unpacker.h"
#include "../converter/RGBConverter.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>

class PXLUnpacker : public Unpacker
{
public:
    PXLUnpacker(std::string filename);

    int unpack();
    bool saveAsPNG(std::string filename);
    void convertToRGB888();

    int width = 320;
    int height = 240;

    int rgb555DataSize = width * height * 2;
    int rgb888DataSize = width * height * 3;

    std::vector<unsigned char> rgb555Data;
    std::vector<unsigned char> rgb888Data;

    std::unique_ptr<RGBConverter> converter;
    bool isConverted = false; 
};

#endif // PXLUNPACKER_H
