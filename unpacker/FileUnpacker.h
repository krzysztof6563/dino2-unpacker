#ifndef FileUnpacker_H
#define FileUnpacker_H

#include "unpacker.h"
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

        bool isConverted = false; 
};

#endif // FileUnpacker_H
