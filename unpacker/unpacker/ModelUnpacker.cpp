#include "ModelUnpacker.h"
#include "DC2ModelExtractor.h"
int ModelUnpacker::unpack() { return DC2ModelExtractor::extract(filename); }
