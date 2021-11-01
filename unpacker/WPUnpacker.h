#ifndef WPUNPACKER_H
#define WPUNPACKER_H

#include "unpacker.h"
#include "../converter/RGBConverter.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>
#include <QDebug>

class WPUnpacker : public Unpacker
{
public:
    WPUnpacker(std::string filename);

    int unpack();
    bool saveAsPNG(std::string filename);

    const int PALLETTE_CHUNK = 33;
    
    int width = 64;

    std::vector<unsigned char> paletteData;
    std::string unpackerName = "WPUnpacker";
};

#endif // WPUNPACKER_H
