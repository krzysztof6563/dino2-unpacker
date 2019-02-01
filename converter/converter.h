#ifndef CONVERTER_H
#define CONVERTER_H


class Converter
{
public:
    Converter();

    virtual int convert() = 0;

protected:
    int dataSize;
    char *data;
};

#endif // CONVERTER_H
