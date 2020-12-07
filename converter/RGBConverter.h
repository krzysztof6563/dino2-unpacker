#ifndef RGBConverter_H
#define RGBConverter_H

#include "converter.h"
#include <stdint.h>
#include <vector>

/**
 * @brief The RGBConverter class, converts palettes used across all files in DC2 from rgb555 to rgb888
 */
class RGBConverter : public Converter {
public:
    RGBConverter();
    ~RGBConverter();

    std::vector<unsigned char> convert(std::vector<unsigned char> data);
    std::vector<unsigned char> unconvert(std::vector<unsigned char> data);

private:
    uint8_t red5, green5, blue5, green25;
    uint8_t red8, green8, blue8;

};

#endif // RGBConverter_H
