#ifndef PALETTECONVERTER_H
#define PALETTECONVERTER_H

#include "converter.h"
#include <stdint.h>

/**
 * @brief The PaletteConverter class, converts palettes used across all files in DC2 from rgb555 to rgb888
 */
class PaletteConverter : public Converter {
public:
    PaletteConverter(int paletteSize);
    ~PaletteConverter();

    int convert();

    char *getNewPalette() const;
    void setNewPalette(char *value);

    char *getPalette() const;
    void setPalette(char *value);

private:
    int BUFFER_SIZE;
    int BREAK_SIZE;
    int PALETTE_SIZE;
    int CONVERTED_PALETTE_SIZE;

    char *palette; //!Pointer to array containing palette to be converted
    char *newPalette; //!Pointer to array used to store coverted palette
    uint8_t red5, green5, blue5, green25;
    uint8_t red8, green8, blue8;

};

#endif // PALETTECONVERTER_H
