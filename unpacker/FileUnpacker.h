#ifndef FileUnpacker_H
#define FileUnpacker_H

#include "unpacker.h"
#include "../converter/RGBConverter.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>

class FileUnpacker : public Unpacker {
    public:
        FileUnpacker(std::string filename);

        int unpack();
        bool saveAsPNG(std::string filename);
        std::string getName();

        std::vector<unsigned char> paletteData;

        std::unique_ptr<RGBConverter> converter;
        bool isConverted = false; 

};

#endif // FileUnpacker_H
