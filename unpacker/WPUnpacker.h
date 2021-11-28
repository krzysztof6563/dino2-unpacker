#ifndef WPUNPACKER_H
#define WPUNPACKER_H

#include "unpacker.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>

class WPUnpacker : public Unpacker {
    public:
        WPUnpacker(std::string filename);

        int unpack();
        bool saveAsPNG(std::string filename);
        std::string getName();

        const int PALLETTE_CHUNK = 33;
        
        int width = 64;
};

#endif // WPUNPACKER_H
