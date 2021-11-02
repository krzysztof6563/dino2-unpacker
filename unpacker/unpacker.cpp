#include "unpacker.h"

Unpacker::Unpacker(std::string filename) {
    this->filename = filename;
    this->inFile.open(filename.c_str(), std::ios::binary);
    dechunker = new Dechunker(filename);
}

Unpacker::~Unpacker() {
    // delete[] buffer;
    // delete[] palette;
    // delete[] newPalette;
    // delete dechunker;
}

void Unpacker::allocateVariables() {
    buffer = new char[BUFFER_SIZE];
    palette = new char[PALETTE_SIZE];
    newPalette = new char[CONVERTED_PALETTE_SIZE];
}

int Unpacker::getBUFFER_SIZE() const {
    return BUFFER_SIZE;
}

void Unpacker::setBUFFER_SIZE(int value) {
    BUFFER_SIZE = value;
}

int Unpacker::getPALETTE_SIZE() const {
    return PALETTE_SIZE;
}

void Unpacker::setPALETTE_SIZE(int value) {
    PALETTE_SIZE = value;
}

int Unpacker::getCONVERTED_PALETTE_SIZE() const {
    return CONVERTED_PALETTE_SIZE;
}

void Unpacker::setCONVERTED_PALETTE_SIZE(int value) {
    CONVERTED_PALETTE_SIZE = value;
}

char *Unpacker::getBuffer() const {
    return buffer;
}

void Unpacker::setBuffer(char *value) {
    buffer = value;
}

char *Unpacker::getPalette() const {
    return palette;
}

char *Unpacker::getNewPalette() const {
    return newPalette;
}

void Unpacker::setNewPalette(char *value) {
    newPalette = value;
}

void Unpacker::convertToRGB888() {
    qDebug() << "Converting RGB555 to RGB888";
    this->rgb888Data = converter->convert(this->rgb555Data);
    this->isConverted = true;
    return;
}