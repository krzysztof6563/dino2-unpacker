#include <iostream>
#include <fstream>
#include <string.h>
#include "unpacker/ItemUnpacker.h"
#include "unpacker/DoorUnpacker.h"
#include "unpacker/DechunkerUnpacker.h"
#include "unpacker/ArmorUnpacker.h"
#include "unpacker/PXLUnpacker.h"
#include "unpacker/WPUnpacker.h"
#include "unpacker/TEXUnpacker.h"
#include "unpacker/StageDBSUnpacker.h"
#include "unpacker/FileUnpacker.h"
#include "unpacker/ComingUnpacker.h"
#include "unpacker/Subscr6Unpacker.h"
#include "dechunker.h"

using namespace std;

void displayHelp() {
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
| ENTRY.DAT    | ❌         | Dino colliseum portraits                                                  |
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
| NO_DATA.PXL  | ❌         |                                                                           |
| OPENING.DAT  | ❌         |                                                                           |
| OPTION.DAT   | ❌         |                                                                           |
| RES*.DAT     | ❌         |                                                                           |
| RESULT.DAT   | ❌         | WAVE files                                                                |
| SAVE.DAT     | ❌         |                                                                           |
| SC*.DAT      | ❌         | WAVE files, RGB555 with pallette                                          |
| ST*.DAT      | ❌         | Mix of files: RGB555 with palletee, WAVE RIFF files and others            |
| ST*.DBS      | ✅         | Saves invidual images as PNGs                                             |
| SUBSCR3.DAT  | ❌         |                                                                           |
| SUBSCR6.DAT  | ❌         | RGB555 image with pallette, contains data for boat ride selection screeen |
| TITLE.DAT    | ❌         | WAVE files                                                                |
| TITLE2.DAT   | ❌         | WAVE files                                                                |
| WARNING.DAT  | ❌         |                                                                           |
| WEP_*.DAT    | ❌         | WAVE files                                                                |
| WP*.DAT      | ✅         | Saves images as PNGs                                                      |
---------------------------------------------------------------------------------------------------------

Copyright Krzysztof Michalski 2019 - 2021
https://github.com/krzysztof6563/dino2-unpacker

Licensed under GPL-3.0 License
https://www.gnu.org/licenses/gpl-3.0.html
)EOL";

}

Unpacker *getUnpackerByFilename(std::string filename) {
    if (filename == "ITEM.DAT") {
        return new ItemUnpacker(filename);
    } else if (filename == "COMING.DAT") {
        return new ComingUnpacker(filename);
    } else if (filename == "FILE.DAT") {
        return new FileUnpacker(filename);
    } else if (filename.find("DOOR") != std::string::npos) {
        return new DoorUnpacker(filename);
    } else if (filename.find("ARMOR") != std::string::npos ||
               filename.find("CAPLOGO.DAT") != std::string::npos ||
            filename.find("MAP.BIN") != std::string::npos) {
        return new ArmorUnpacker(filename);
    } else if (filename.find(".PXL") != std::string::npos) {
        return new PXLUnpacker(filename);
    } else if (filename.find("WP") != std::string::npos) {
        return new WPUnpacker(filename);
    } else if (filename.find(".TEX") != std::string::npos) {
        return new TEXUnpacker(filename);
    } else if (filename.find(".DBS") != std::string::npos) {
        return new StageDBSUnpacker(filename);
    } else if (filename.find("SUBSCR6.DAT") != std::string::npos) {
        return new Subscr6Unpacker(filename);
    } else {
        return new DechunkerUnpacker(filename);
    }
//    return nullptr;
}

struct GlobalOptions {
    bool saveChunks = false;
    bool displayHelp = false;
};

void setupGlobalOptions (struct GlobalOptions *options, std::vector<std::string> *cliOptions) {
    for (std::string option : *cliOptions) {
        if (option == "--help") options->displayHelp = true;
        if (option == "--save-chunks") options->saveChunks = true;
    }
}


int main(int argc, char *argv[]) {
    std::vector<std::string> filenames;
    std::vector<std::string> options;
    GlobalOptions globalOptions;

    for (int i=1; i<argc; i++) {
        std::string arg(argv[i]);

        if (arg.substr(0, 2) == "--") {
            options.push_back(arg);
        } else {
            filenames.push_back(arg);
        }
    }

    setupGlobalOptions(&globalOptions, &options);

    if (globalOptions.displayHelp || argc <= 1 || filenames.empty()) {
        displayHelp();
        
        return 0;
    }

    for (std::string inputFile : filenames) {
        if (std::filesystem::exists(inputFile) && std::filesystem::is_regular_file(inputFile)) {
            Unpacker *u = getUnpackerByFilename(inputFile);
            if (u != nullptr) {
                std::cout << "[INFO] Choosen Unpacker type: " << u->getName() << std::endl;
                u->unpack();
                std::cout << "[INFO] Unpacked: " << inputFile << std::endl;

                if (globalOptions.saveChunks) {
                    u->saveChunks();
                }
            } else {
                std::cout << "[ERROR] File type not supported" << std::endl;
            }
            delete u;
        } else {
            std::cout << "[ERROR] File " << inputFile << " not found" << std::endl;
        }
    }

    return 0;
    }
