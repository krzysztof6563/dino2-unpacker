#include "ItemUnpacker.h"

ItemUnpacker::ItemUnpacker(std::string filename) : Unpacker (filename) {
    this->converter = std::unique_ptr<RGBConverter>(new RGBConverter());
}

ItemUnpacker::~ItemUnpacker() {
}

int ItemUnpacker::unpack() {
    if (inFile.is_open()) {
        std::cout << "File" << filename << " opened." << std::endl;
        dechunker->dechunk();
        size_t total = dechunker->getNumberOfChunks();
        int imageNumber = 0;
        if (!std::filesystem::is_directory("ITEM_pngs")){
            std::filesystem::create_directory("ITEM_pngs");
        }
        for (size_t i = 0; i < total; i += 5, imageNumber++) {
            // char output[255];
            outData.clear();
            //add check if file is opened
            // sprintf(output, "%s%04d.data", filename.c_str(), imageNumber);
            // outFile.open(output, std::fstream::binary | std::fstream::trunc);
            // printf("Creating %s\n", output);
            for (size_t j = 0; j < 4; j+=2) {
                auto chunk = dechunker->getChunkAt(i+j);
                auto chunk2 = dechunker->getChunkAt(i+j+1);
                outFile.write(chunk, dechunker->getChunkSize());
                outFile.write(chunk2, dechunker->getChunkSize());
                for (size_t k = 0; k < 64; k++) {
                    char* activeChunk = chunk;
                    if (k%2 !=0) {
                        activeChunk = chunk2;
                    }
                    for (size_t l = 0; l < 64; l++) {
                        this->outData.push_back(activeChunk[k/2*64 + l]);
                    }
                }
            }

            // char paletteOutput[255];
            // sprintf(paletteOutput, "%s%04d.data.pal", filename.c_str(), imageNumber);
            // printf("Creating %s\n", paletteOutput);
            // outFilePalette.open(paletteOutput, std::fstream::binary | std::fstream::trunc);
            char* chunk = dechunker->getChunkAt(i+4);
            this->paletteData.clear();
            for (size_t i = 0; i < dechunker->getChunkSize(); i++)
            {
                this->paletteData.push_back(chunk[i]);
            }


            paletteData = this->converter->convert(paletteData);

            // for (size_t i = 0; i < paletteData.size(); i++)
            // {
            //     outFilePalette.write((char *)&paletteData.at(i), sizeof paletteData.at(i));
            // }
            
            
            // outFile.close();
            // outFilePalette.close();
            
            if (this->saveAsPNG("ITEM_pngs/" + this->filename + std::to_string(imageNumber))) {
                std::cout << "[INFO] Saved as ITEM_pngs/" << this->filename << std::to_string(imageNumber) << std::endl;
            } else {
                std::cout << "[ERROR] Can't save as ITEM_pngs/" << this->filename << std::to_string(imageNumber) << std::endl;
            }
        }
        inFile.close();

    } else {
        std::cout << "Error opening  " << filename;
    }
    return 0;
}

bool ItemUnpacker::saveAsPNG(std::string outFileName) {
    auto image = new QImage(&outData[0], this->width, this->height, QImage::Format::Format_Indexed8, nullptr, nullptr);
    this->colors.clear();
    for (size_t i = 0; i<paletteData.size(); i += 3) {
        auto color = QColor(paletteData.at(i), paletteData.at(i + 1), paletteData.at(i + 2));
        this->colors.push_back(color.rgb());
    }
    image->setColorTable(this->colors);

    return image->save(QString::fromStdString(outFileName+".png"), "PNG");
}

std::string ItemUnpacker::getName() {
    return "ItemUnpacker";
}