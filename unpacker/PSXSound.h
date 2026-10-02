#pragma once

#include <string>

namespace dc2 {

// PlayStation sound banks: a type-3 "Gian" instrument header followed by a type-4 entry of
// SPU ADPCM sample data (no offsets are stored; each sample ends with an end-flagged
// block). Writes every sample as <file>.<n>.WAV, 16-bit mono at 22050 Hz, the same
// recordings and numbering the PC version's WAV banks have. Returns the number written.
int extractPlayStationSounds(const std::string& filename);

}
