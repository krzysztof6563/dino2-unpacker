#pragma once

#include <iostream>

class Help {
    public:
        static void displayHelp() {
    std::cout << R"EOL(
Usage: dino2-unpacker [OPTIONS] FILE...

Supported file formats:

|-------------------------------------------------------------------------------------------------------|
| FILE PATTERN | SUPPORTED | NOTES                                                                      |
| ------------ | --------- | ---------------------------------------------------------------------------|
| *.PXL        | ✅         | Saves images as PNGs                                                      |
| *.TEX        | ✅         | Saves images as PNGs                                                      |
| ARMOR*.DAT   | 🔨         | Implemented dechunking                                                    |
| CAPLOGO.DAT  | ❌         |                                                                           |
| COMING.DAT   | ✅         | RGB555 image with pallette                                                |
| CONTINUE.DAT | ❌         |                                                                           |
| CORE*.DAT    | ❌         | WAVE files                                                                |
| DEMO*.TRG    | ❌         |                                                                           |
| DOOR*.DAT    | 🔨         | Extracts textures and soudns, 3D model missing                            |
| E*.DAT       | ❌         | Enemy data 3D models, WAVE, RGB555 textures (optional)                    |
| ENDING.DAT   | ❌         | WAVE files                                                                |
| ENTRY.DAT    | 🔨         | Dino colliseum portraits, some data after                                 |
| FILE.DAT     | ✅         | Dino File images stored as RGB555 with pallette                           |
| GAMEOVER.DAT | ❌         |                                                                           |
| ITEM.DAT     | ✅         | Saves images as PNGs                                                      |
| KOF_*.DAT    | ❌         | Sound effects in WAVE format                                              |
| LOAD.DAT     | ❌         |                                                                           |
| MAP.BIN      | ❌         |                                                                           |
| MAP.DAT      | ❌         |                                                                           |
| MAP_ST*.DAT  | ❌         | RGB555 images with pallette                                               |
| ME_*.DAT     | ❌         | LAME MP3 Files (Enemy Music)                                              |
| MF_*.DAT     | ❌         | LAME MP3 Files (Background Music)                                         |
| M_RESULT.DAT | ❌         | Possibly LAME MP3 Files                                                   |
| MS_*.DAT     | ❌         | LAME MP3 Files (Background Sounds)                                        |
| M_TITLE.DAT  | ❌         | WAVE files at bottom                                                      |
| OPENING.DAT  | ❌         |                                                                           |
| OPTION.DAT   | ❌         |                                                                           |
| RES*.DAT     | ❌         |                                                                           |
| RESULT.DAT   | ❌         | WAVE files                                                                |
| SAVE.DAT     | ❌         |                                                                           |
| SC*.DAT      | ❌         | WAVE files, RGB555 with pallette                                          |
| ST*.DAT      | ❌         | Mix of files: RGB555 with palletee, WAVE RIFF files and others            |
| ST*.DBS      | ✅         | Saves invidual images as JPGs                                             |
| SUBSCR3.DAT  | ❌         |                                                                           |
| SUBSCR6.DAT  | ✅         | RGB555 image with pallette, contains data for boat ride selection screeen |
| TITLE.DAT    | ❌         | WAVE files                                                                |
| TITLE2.DAT   | ❌         | WAVE files                                                                |
| WARNING.DAT  | ❌         |                                                                           |
| WEP_*.DAT    | ❌         | WAVE files                                                                |
| WP*.DAT      | 🔨         | Saves images as PNGs, additional data after images for 75A, 79A, 83A, 84A |
---------------------------------------------------------------------------------------------------------

Copyright Krzysztof Michalski 2019 - 2021
https://github.com/krzysztof6563/dino2-unpacker

Licensed under GPL-3.0 License
https://www.gnu.org/licenses/gpl-3.0.html
)EOL";
    }
};