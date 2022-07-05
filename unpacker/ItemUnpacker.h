#ifndef ITEMUNPACKER_H
#define ITEMUNPACKER_H

#include <cstring>
#include "unpacker.h"
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>
#include <QVector>

class ItemUnpacker : public Unpacker {
    public:
        ItemUnpacker(std::string filename);
        ~ItemUnpacker();

        int unpack();
        std::string getName();
};

#endif // ITEMUNPACKER_H
