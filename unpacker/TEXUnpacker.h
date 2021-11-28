#ifndef TEXUNPACKER_H
#define TEXUNPACKER_H

#include "unpacker.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>

class TEXUnpacker : public Unpacker {
    public:
        TEXUnpacker(std::string filename);

        int unpack();
        bool saveAsPNG(std::string filename, int offset, int imageHeight);
        std::string getName();

        const int PALLETTE_CHUNK = 33;
        
        int width = 64;
        bool isConverted = false; 

};

#endif // TEXUNPACKER_H
