#include "paletteconverter.h"

PaletteConverter::PaletteConverter(int paletteSize) : PALETTE_SIZE(paletteSize)
{

}

PaletteConverter::~PaletteConverter()
{

}

int PaletteConverter::convert()
{
    int newI = 0;
    for (int i=0; i<PALETTE_SIZE; i+=2) {
        char byte1 = palette[i];
        char byte2 = palette[i+1];
        red5 = byte1 & 0b00011111;
        blue5 = byte2 & 0b01111100;
        blue5 >>= 2;
        green5 = byte2 & 0b00000011;
        green5 <<= 3;
        green25 = byte1 & 0b11100000;
        green25 >>= 5;
        green5 = green5 | green25;
        red8 = ((red5 * 527) + 23) >> 6;
        green8 = ((green5 * 527) + 23) >> 6;
        blue8 = ((blue5 * 527) + 23) >> 6;

        newPalette[newI] = red8;
        newPalette[newI+1] = green8;
        newPalette[newI+2] = blue8;
        newI += 3;
    }
}

char *PaletteConverter::getNewPalette() const
{
    return newPalette;
}

void PaletteConverter::setNewPalette(char *value)
{
    newPalette = value;
}

char *PaletteConverter::getPalette() const
{
    return palette;
}

void PaletteConverter::setPalette(char *value)
{
    palette = value;
}
