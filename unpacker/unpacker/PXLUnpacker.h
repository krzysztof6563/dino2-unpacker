#ifndef PXLUNPACKER_H
#define PXLUNPACKER_H

#include "unpacker.h"
#include <string>
#include <vector>
#include <QPixmap>
#include <QString>

class PXLUnpacker : public Unpacker {
    public:
        PXLUnpacker(std::string filename);

        int unpack();
    std::string getName();
};

#endif // PXLUNPACKER_H
