#ifndef ITEMUNPACKER_H
#define ITEMUNPACKER_H

#include <cstring>
#include "unpacker.h"
#include "../converter/paletteconverter.h"
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>
#include <QDebug>
#include <QVector>

class ItemUnpacker : public Unpacker {
public:
    ItemUnpacker(std::string filename);
    ~ItemUnpacker();

    int unpack();
    bool saveAsPNG(std::string outFileName, char* palette);

    std::vector<unsigned char> outData;
    QVector<QRgb> colors;
    std::string unpackerName = "ItemUnpacker";

private:
    PaletteConverter *p;
};

#endif // ITEMUNPACKER_H
