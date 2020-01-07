#ifndef ITEMUNPACKER_H
#define ITEMUNPACKER_H

#include <cstring>
//#include <Magick++.h>
#include "unpacker.h"
#include "../converter/paletteconverter.h"

class ItemUnpacker : public Unpacker {
public:
    ItemUnpacker(std::string filename);
    ~ItemUnpacker();

    int unpack();

private:
    PaletteConverter *p;
};

#endif // ITEMUNPACKER_H
