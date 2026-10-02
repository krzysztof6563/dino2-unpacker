#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "help.h"
#include "UnpackerChooser.h"
#include "unpacker/DC2ModelExtractor.h"
#include "unpacker/DC2Platform.h"
#include "unpacker/PSXSound.h"

namespace fs = std::filesystem;

struct GlobalOptions {
    bool saveChunks = false;
    bool displayHelp = false;
    fs::path outputDirectory;
    dc2::AnimationOptions animations;
};

// Returns false (after printing why) for an unknown or malformed option.
bool setupGlobalOptions(GlobalOptions& options, const std::vector<std::string>& cliOptions) {
    for (const std::string& option : cliOptions) {
        if (option == "--help") options.displayHelp = true;
        else if (option == "--save-chunks") options.saveChunks = true;
        else if (option == "--no-animations") options.animations.enabled = false;
        else if (option == "--in-place") options.animations.inPlace = true;
        else if (option.rfind("--output-dir=", 0) == 0) options.outputDirectory = option.substr(13);
        else if (option.rfind("--animation-tps=", 0) == 0) {
            try {
                std::size_t used = 0;
                const std::string value = option.substr(16);
                options.animations.ticksPerSecond = std::stod(value, &used);
                if (used != value.size() || !std::isfinite(options.animations.ticksPerSecond) ||
                    options.animations.ticksPerSecond <= 0 || options.animations.ticksPerSecond > 1000000)
                    throw std::invalid_argument(value);
            } catch (const std::exception&) {
                std::cout << "[ERROR] Invalid animation tick rate: " << option << '\n';
                return false;
            }
        } else {
            std::cout << "[ERROR] Unknown option: " << option << " (see --help)\n";
            return false;
        }
    }
    return true;
}

// Extracts one file in place, next to itself.
void unpackFile(const std::string& inputFile, const GlobalOptions& options) {
    const dc2::GameVersion version = dc2::detectVersion(inputFile);
    std::cout << "[INFO] Game version: " << dc2::versionName(version) << '\n';
    Unpacker *u = UnpackerChooser::getUnpackerByFilename(inputFile);
    if (u == nullptr) {
        std::cout << "[ERROR] File type not supported" << '\n';
        return;
    }
    std::cout << "[INFO] Choosen Unpacker type: " << u->getName() << '\n';
    u->findAndExtractRIFFFiles();
    if (version == dc2::GameVersion::PlayStation)
        dc2::extractPlayStationSounds(inputFile);
    u->unpack();
    std::cout << "[INFO] Unpacked: " << inputFile << '\n';
    if (options.saveChunks) {
        u->saveChunks();
    }
    delete u;
}

int main(int argc, char *argv[]) {
    std::vector<std::string> inputs;
    std::vector<std::string> cliOptions;
    GlobalOptions options;

    for (int i=1; i<argc; i++) {
        std::string arg(argv[i]);

        if (arg.substr(0, 2) == "--") {
            cliOptions.push_back(arg);
        } else {
            inputs.push_back(arg);
        }
    }

    if (!setupGlobalOptions(options, cliOptions)) return 1;

    if (options.displayHelp || inputs.empty()) {
        Help::displayHelp();
        return 0;
    }
    DC2ModelExtractor::setAnimationOptions(options.animations);
    DC2ModelExtractor::setSaveArchiveEntries(options.saveChunks);

    // Folders are searched recursively; each file is handled once.
    std::vector<fs::path> files;
    for (const std::string& input : inputs) {
        if (fs::is_directory(input)) {
            std::vector<fs::path> found;
            for (const auto& item : fs::recursive_directory_iterator(input))
                if (item.is_regular_file()) found.push_back(item.path());
            std::sort(found.begin(), found.end());
            files.insert(files.end(), found.begin(), found.end());
        } else if (fs::is_regular_file(input)) {
            files.push_back(input);
        } else {
            std::cout << "[ERROR] File " << input << " not found" << '\n';
        }
    }

    std::set<fs::path> seen, used;
    for (const fs::path& file : files) {
        const fs::path source = fs::absolute(file).lexically_normal();
        if (!seen.insert(source).second) continue;
        if (options.outputDirectory.empty()) {
            unpackFile(source.string(), options);
        } else {
            // Unpack a copy inside the output folder, so every unpacker writes there; files
            // with the same name from different folders get a numbered subfolder.
            fs::path folder = options.outputDirectory;
            for (int n = 1; used.count(folder / source.filename()); ++n)
                folder = options.outputDirectory / std::to_string(n);
            used.insert(folder / source.filename());
            const fs::path copy = folder / source.filename();
            std::error_code error;
            fs::create_directories(folder, error);
            fs::copy_file(source, copy, fs::copy_options::overwrite_existing, error);
            if (error) {
                std::cout << "[ERROR] Cannot write to " << folder.string() << ": " << error.message() << '\n';
                continue;
            }
            DC2ModelExtractor::setSourceDirectory(source.parent_path().string());
            unpackFile(copy.string(), options);
            fs::remove(copy, error);
        }
        std::cout << "\n";
    }

    return 0;
}
