#include "RGBConverter.h"

RGBConverter::RGBConverter() {

};

RGBConverter::~RGBConverter() {

}

/**
 * @brief RGBConverter::convert Cenverts rgb555 palette to rgb888
 * @return void
 */
std::vector<unsigned char> RGBConverter::convert(std::vector<unsigned char> data) {
    std::vector<unsigned char> newData;
    uint8_t red5, green5, blue5, green25;
    uint8_t red8, green8, blue8;
    char byte1, byte2;

    for (std::size_t i=0; i<data.size(); i+=2) {
        //Reading 2 bytes
        byte1 = data.at(i);
        byte2 = data.at(i+1);
        //Extracting 5 red bits
        red5 = byte1 & 0b00011111;
        //Extracting 5 blue bits
        blue5 = byte2 & 0b01111100;
        blue5 >>= 2;
        //Extracting 5 green bits
        green5 = byte2 & 0b00000011;
        green5 <<= 3;
        green25 = byte1 & 0b11100000;
        green25 >>= 5;
        green5 = green5 | green25;
        //Converting to 8 bit
        red8 = ((red5 * 527) + 23) >> 6;
        green8 = ((green5 * 527) + 23) >> 6;
        blue8 = ((blue5 * 527) + 23) >> 6;
        //Writing to file
        newData.push_back(red8);
        newData.push_back(green8);
        newData.push_back(blue8);
    }

    return newData;
}

/**
 * @brief RGBConverter::convert Cenverts rgb888 palette to rgb555
 * @return void
 */
std::vector<unsigned char> RGBConverter::unconvert(std::vector<unsigned char> data) {
    // int newI = 0;
    // for (int i=0; i<PALETTE_SIZE; i+=2) {
    //     char byte1 = palette[i];
    //     char byte2 = palette[i+1];
    //     red5 = byte1 & 0b00011111;
    //     blue5 = byte2 & 0b01111100;
    //     blue5 >>= 2;
    //     green5 = byte2 & 0b00000011;
    //     green5 <<= 3;
    //     green25 = byte1 & 0b11100000;
    //     green25 >>= 5;
    //     green5 = green5 | green25;
    //     red8 = ((red5 * 527) + 23) >> 6;
    //     green8 = ((green5 * 527) + 23) >> 6;
    //     blue8 = ((blue5 * 527) + 23) >> 6;

    //     newPalette[newI] = red8;
    //     newPalette[newI+1] = green8;
    //     newPalette[newI+2] = blue8;
    //     newI += 3;
    // }
    // return 1;
    std::vector<unsigned char> test;
    return test;
}