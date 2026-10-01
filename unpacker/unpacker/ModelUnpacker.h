#pragma once
#include "unpacker.h"
class ModelUnpacker : public Unpacker {
public:
    ModelUnpacker(std::string filename) : Unpacker(filename) {}
    int unpack() override;
    std::string getName() override { return "ModelUnpacker"; }
};
