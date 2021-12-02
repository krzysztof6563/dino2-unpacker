#ifndef COMINGUNPACKER_H
#define COMINGUNPACKER_H

#include "unpacker.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>

class ComingUnpacker : public Unpacker {
    public:
        ComingUnpacker(std::string filename);

        int unpack();
        bool saveAsPNG(std::string filename);
        std::string getName();

        const int PALLETTE_CHUNK = 33;
        
        int width = 64;
};

#endif // ComingUnpacker_H
