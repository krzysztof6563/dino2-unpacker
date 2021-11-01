#ifndef UNPACKER_H
#define UNPACKER_H

#include <fstream>
#include <iostream>
#include <string>
#include "../converter/RGBConverter.h"
#include "../dechunker.h"
#include <QDebug>
#include <QVector>
#include <QRgb>

/**
 * @brief The Unpacker class, virutal class for all unpackers
 */
class Unpacker {
public:
    Unpacker(std::string filename);
    virtual ~Unpacker();

    virtual int unpack() = 0;
    virtual void allocateVariables();

    int getBUFFER_SIZE() const;
    void setBUFFER_SIZE(int value);

    int getPALETTE_SIZE() const;
    void setPALETTE_SIZE(int value);

    int getCONVERTED_PALETTE_SIZE() const;
    void setCONVERTED_PALETTE_SIZE(int value);

    int getTILE_SIZE() const;
    void setTILE_SIZE(int value);

    int getTILE_WIDTH() const;
    void setTILE_WIDTH(int value);

    char *getBuffer() const;
    void setBuffer(char *value);

    char *getPalette() const;

    char *getNewPalette() const;
    void setNewPalette(char *value);

    std::ifstream getInFile() const;
    void setInFile(const std::ifstream &value);

    std::ofstream getOutFile() const;
    void setOutFile(const std::ofstream &value);

    std::ofstream getOutFilePalette() const;
    void setOutFilePalette(const std::ofstream &value);

    std::string unpackerName;

protected:
    int BUFFER_SIZE;
    int PALETTE_SIZE;
    int CONVERTED_PALETTE_SIZE;
//    int TILE_SIZE;
//    int TILE_WIDTH;

    std::string filename;

    char *buffer;
    char *palette;
    char *newPalette;

    std::ifstream inFile;
    std::ofstream outFile;
    std::ofstream outFilePalette;

    Dechunker *dechunker;

    void convertToRGB888();

    std::vector<unsigned char> rgb555Data;
    std::vector<unsigned char> rgb888Data;
    QVector<QRgb> colors;
    std::unique_ptr<RGBConverter> converter;
    bool isConverted = false; 

};

#endif // UNPACKER_H
