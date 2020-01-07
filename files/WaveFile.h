#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <ios>

class WaveFile {
public:
    WaveFile();
    WaveFile(std::streampos start) {
        this->start = start;
    };
    WaveFile(std::streampos start, unsigned long long int size) {
        this->start = start;
        this->size = size;
    };

    std::streampos getStart()
    {
        return this->start;
    }

    void setStart(std::streampos start)
    {
        this->start = start;
    }

    unsigned long long int getSize()
    {
        return this->size;
    }

    void setSize(unsigned long long int size)
    {
        this->size = size;
    }

private:
    std::streampos start;
    unsigned long long int size;
};

#endif //WAVEFILE_H
