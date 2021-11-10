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
        bool saveAsPNG(std::string outFileName);
        std::string getName();

        std::vector<unsigned char> outData;
        QVector<QRgb> colors;
        std::unique_ptr<RGBConverter> converter;
        std::vector<unsigned char> paletteData;

    private:
        PaletteConverter *p;
        int width = 64*2;
        int height = 2*32;

};

#endif // ITEMUNPACKER_H
