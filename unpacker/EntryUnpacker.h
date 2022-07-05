#ifndef EntryUnpacker_H
#define EntryUnpacker_H

#include "unpacker.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>

class EntryUnpacker : public Unpacker {
    public:
        EntryUnpacker(std::string filename);

        int unpack();
        std::string getName();

        bool isConverted = false; 
};

#endif // EntryUnpacker_H
