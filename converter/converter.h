#ifndef CONVERTER_H
#define CONVERTER_H

/**
 * @brief The Converter class, virutal class for all converteers
 */
class Converter
{
public:
    Converter();

    virtual int convert() = 0;

protected:
    int dataSize; //!< Size of data to be processed
    char *data; //!< Data to be processed
};

#endif // CONVERTER_H
