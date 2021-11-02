#ifndef CONVERTER_H
#define CONVERTER_H

#include <vector>

/**
 * @brief The Converter class, virutal class for all converteers
 */
class Converter {
public:
    Converter();

    virtual std::vector<unsigned char> convert(std::vector<unsigned char> palette) = 0;
    virtual std::vector<unsigned char> unconvert(std::vector<unsigned char> palette) = 0;

protected:
    int dataSize; //!< Size of data to be processed
    char *data; //!< Data to be processed
};

#endif // CONVERTER_H
