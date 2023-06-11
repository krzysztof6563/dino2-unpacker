#include <iostream>
#include <fstream>
#include <string.h>
#include "help.h"
#include "UnpackerChooser.h"

using namespace std;

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
        Help::displayHelp();
        
        return 0;
    }

    for (std::string inputFile : filenames) {
        if (std::filesystem::exists(inputFile) && std::filesystem::is_regular_file(inputFile)) {
            Unpacker *u = UnpackerChooser::getUnpackerByFilename(inputFile);
            if (u != nullptr) {
                std::cout << "[INFO] Choosen Unpacker type: " << u->getName() << '\n';
                u->findAndExtractRIFFFiles();
                u->unpack();
                std::cout << "[INFO] Unpacked: " << inputFile << '\n';

                if (globalOptions.saveChunks) {
                    u->saveChunks();
                }
            } else {
                std::cout << "[ERROR] File type not supported" << '\n';
            }
            delete u;
        } else {
            std::cout << "[ERROR] File " << inputFile << " not found" << '\n';
        }

        std::cout << "\n";
    }

    return 0;
    }
