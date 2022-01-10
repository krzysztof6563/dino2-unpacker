#ifndef Subscr6Unpacker_H
#define Subscr6Unpacker_H

#include "unpacker.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>

class Subscr6Unpacker : public Unpacker {
    public:
        Subscr6Unpacker(std::string filename);

        int unpack();
        bool saveAsPNG(std::string filename);
        std::string getName();

        const int PALLETTE_CHUNK = 50;
        
        int width = 64;
};

#endif // Subscr6Unpacker_H
