#include "StageDBSUnpacker.h"

StageDBSUnpacker::StageDBSUnpacker(std::string filename) : Unpacker (filename) {}

StageDBSUnpacker::~StageDBSUnpacker() {}

int StageDBSUnpacker::unpack() {
    if (inFile.is_open()) {
        this->dechunker->dechunk();
        size_t total = dechunker->getNumberOfChunks();
        int imageNumber = 0;

        for (int i=0; i<dechunker->getNumberOfChunks(); i++) {
            if (this->isChunkJPEGStart(dechunker->getChunkAt(i))) {
                jpegStartPoints.push_back(i);
            }
        }

        if (jpegStartPoints.size() != 0) {
            if (!std::filesystem::is_directory(filename+"_jpgs")){
                std::filesystem::create_directory(filename+"_jpgs");
            }
            jpegStartPoints.push_back(dechunker->getNumberOfChunks());
        } else {
            // The PlayStation version stores the same backgrounds as MDEC frames.
            const int decoded = extractPlayStationBackgrounds();
            if (decoded == 0) std::cout << "[INFO] No JPEG files or MDEC frames found" << '\n';
            inFile.close();
            return decoded >= 0 ? 0 : 1;
        }

        for (size_t i = 0; i < jpegStartPoints.size() - 1; i++) {
            size_t startIndex = jpegStartPoints.at(i);
            std::cout << "[DEBUG] JPEG start found in chunk " << startIndex << '\n';

            std::ostringstream newName;
            newName.fill('0');
            newName << filename << "_jpgs/" << std::setw(2) << i << ".jpg";
            std::string outFilename = newName.str();
            
            outFile.open(outFilename, std::fstream::binary);
            for (size_t j = startIndex; j < jpegStartPoints[i+1]; j++) {
                outFile.write(dechunker->getChunkAt(j), dechunker->getChunkSize());
            }
            outFile.close();
            
            std::cout << "[INFO] JPEG saved as " << outFilename << '\n';
        }
        
        // if (!std::filesystem::is_directory("ITEM_u")){
        //     std::filesystem::create_directory("ITEM_u");
        // }
        
        inFile.close();
        std::cout << "[DEBUG] Closed file " << filename << '\n';

    } else {
        std::clog << "Error opening  " << filename;
    }
    return 0;
}

/**
 * PlayStation backgrounds: 320x240 MDEC "BS" frames, each starting on a sector boundary.
 * Saved as <file>_pngs/NN.png, numbered like the PC version's JPEGs. Returns the number
 * decoded, or -1 if a frame could not be decoded.
 */
int StageDBSUnpacker::extractPlayStationBackgrounds() {
    std::ifstream input(filename, std::ios::binary);
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(input)), {});
    int count = 0;
    bool failed = false;
    for (std::size_t offset = 0; offset + 0x800 <= data.size(); offset += 0x800) {
        if (!dc2::isBsFrame(data.data() + offset, data.size() - offset)) continue;
        std::vector<std::uint8_t> rgb;
        if (!dc2::decodeBsFrame(data.data() + offset, data.size() - offset, BACKGROUND_WIDTH, BACKGROUND_HEIGHT, rgb)) {
            std::cout << "[WARNING] Could not decode the MDEC frame at 0x" << std::hex << offset << std::dec << '\n';
            failed = true;
            continue;
        }
        if (!std::filesystem::is_directory(filename + "_pngs")) {
            std::filesystem::create_directory(filename + "_pngs");
        }
        std::ostringstream name;
        name.fill('0');
        name << filename << "_pngs/" << std::setw(2) << count++ << ".png";
        const QImage image(rgb.data(), BACKGROUND_WIDTH, BACKGROUND_HEIGHT, BACKGROUND_WIDTH * 3, QImage::Format_RGB888);
        if (image.save(QString::fromStdString(name.str()), "PNG")) {
            std::cout << "[INFO] PlayStation background saved as " << name.str() << '\n';
        } else {
            failed = true;
        }
    }
    return failed ? -1 : count;
}

bool StageDBSUnpacker::isChunkJPEGStart(char* chunk) {
    for (size_t i = 0; i < JPEG_START_LENGTH; i++) {
        if ((unsigned char)chunk[i] != JPEG_START[i]) {
            return false;
        }
    }

    return true;
}

std::string StageDBSUnpacker::getName() {
    return "StageDBSUnpacker";
}