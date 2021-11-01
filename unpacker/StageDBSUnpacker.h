#ifndef StageDBSUnpacker_H
#define StageDBSUnpacker_H

#include <cstring>
#include "unpacker.h"
#include "../converter/paletteconverter.h"
#include <vector>
#include <QPixmap>
#include <QString>
#include <QRgb>
#include <QDebug>
#include <QVector>

class StageDBSUnpacker : public Unpacker {
public:
    StageDBSUnpacker(std::string filename);
    ~StageDBSUnpacker();

    int unpack();
    bool saveAsPNG(std::string outFileName, char* palette);

    std::vector<unsigned char> outData;
    QVector<QRgb> colors;
    std::string unpackerName = "StageDBSUnpacker";


private:
    PaletteConverter *p;
    const int JPEG_START_LENGTH = 12;
    const unsigned char JPEG_START[12] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01};
    std::vector<size_t> jpegStartPoints;

    bool isChunkJPEGStart(char* chunk);
};

#endif // StageDBSUnpacker_H
