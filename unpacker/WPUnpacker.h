#ifndef WPUNPACKER_H
#define WPUNPACKER_H

#include "unpacker.h"
#include "../converter/RGBConverter.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>

class WPUnpacker : public Unpacker
{
public:
    WPUnpacker(std::string filename);

    int unpack();
    bool saveAsPNG(std::string filename);
    void convertToRGB888();

    const int PALLETTE_CHUNK = 33;
    
    int width = 64;

    std::vector<unsigned char> rgb555Data;
    std::vector<unsigned char> rgb888Data;
    std::vector<unsigned char> paletteData;
    QVector<QRgb> colors;

    std::unique_ptr<RGBConverter> converter;
    bool isConverted = false; 
};

#endif // WPUNPACKER_H
