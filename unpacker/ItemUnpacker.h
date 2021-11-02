#ifndef ITEMUNPACKER_H
#define ITEMUNPACKER_H

#include <cstring>
#include "unpacker.h"
#include "../converter/paletteconverter.h"
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
        bool saveAsPNG(std::string outFileName, char* palette);
        std::string getName();

        std::vector<unsigned char> outData;
        QVector<QRgb> colors;

    private:
        PaletteConverter *p;
};

#endif // ITEMUNPACKER_H
