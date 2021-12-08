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
#include <QImage>
#include <QColor>

/**
 * @brief The Unpacker class, virutal class for all unpackers
 */
class Unpacker {
public:
    Unpacker(std::string filename);
    virtual ~Unpacker();

    virtual int unpack() = 0;
    virtual std::string getName() = 0;
    // virtual bool saveAsPng(std::string outFileName);

    char *getBuffer() const;
    void setBuffer(char *value);

    char *getPalette() const;

    char *getNewPalette() const;
    void setNewPalette(char *value);

    std::ifstream getInFile() const;
    void setInFile(const std::ifstream &value);

    std::ofstream getOutFile() const;
    void setOutFile(const std::ofstream &value);

    std::ofstream gesaveChunkstOutFilePalette() const;
    void setOutFilePalette(const std::ofstream &value);

    bool saveChunks();

protected:
    int PNG_WIDTH = 64;
    int PNG_HEIGHT = 64;

    std::string filename;

    char *buffer;
    char *palette;
    char *newPalette;

    std::ifstream inFile;
    std::ofstream outFile;
    std::ofstream outFilePalette;

    Dechunker *dechunker;

    void convertToRGB888();
    bool saveAsIndexedPNG(std::string fileNameBase, int offset = 0);
    bool saveAsRGB888PNG(std::string fileNameBase, int offset = 0);
    void loadChunksToRGB555Vector();
    std::vector<uint8_t> rearrangeChunks(int width, int height, int start, std::vector<uint8_t> initialData);

    std::vector<uint8_t> rgb555Data;
    std::vector<uint8_t> rgb888Data;
    std::vector<uint8_t> paletteData;
    QVector<QRgb> colors;
    bool isConverted = false; 

    std::unique_ptr<RGBConverter> converter;


};

#endif // UNPACKER_H
