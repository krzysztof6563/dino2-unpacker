#ifndef PXLUNPACKER_H
#define PXLUNPACKER_H

#include "unpacker.h"
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

    int width = 320;
    int height = 240;

    int rgb555DataSize = width * height * 2;
    int rgb888DataSize = width * height * 3;
    std::string unpackerName = "PXLUnpacker";
};

#endif // PXLUNPACKER_H
