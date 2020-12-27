#include "ItemUnpacker.h"

ItemUnpacker::ItemUnpacker(std::string filename) : Unpacker (filename)
{
    std::cout << "** Unpacker type: ItemUnpacker" << std::endl;
    BUFFER_SIZE = 64*32*4;
    PALETTE_SIZE = 64*8+64*24;
    CONVERTED_PALETTE_SIZE = PALETTE_SIZE/2*3;
    this->allocateVariables();
    p = new PaletteConverter(PALETTE_SIZE);
    
}

ItemUnpacker::~ItemUnpacker()
{
    delete p;
}

int ItemUnpacker::unpack()
{
    if (inFile.is_open()) {
        std::cout << "File" << filename << " opened." << std::endl;
        dechunker->dechunk();
        size_t total = dechunker->getNumberOfChunks();
        int imageNumber = 0;
        if (!std::filesystem::is_directory("ITEM_u")){
            std::filesystem::create_directory("ITEM_u");
        }
        for (size_t i = 0; i < total; i += 5, imageNumber++) {
            char output[255];
            outData.clear();
            //add check if file is opened
            sprintf(output, "%s%04d.data", filename.c_str(), imageNumber);
            outFile.open(output, std::fstream::binary);
            printf("Creating %s\n", output);
            for (size_t j = 0; j < 4; ++j) {
                auto chunk = dechunker->getChunkAt(i+j);
                outFile.write(chunk, dechunker->getChunkSize());
                for (size_t k = 0; k < this->dechunker->getChunkSize(); k++)
                {
                    outData.push_back(chunk[k]);
                }
                
            }
            char paletteOutput[255];
            sprintf(paletteOutput, "%s%04d.data.pal", filename.c_str(), imageNumber);
            outFilePalette.open(paletteOutput, std::fstream::binary);
            printf("Creating %s\n", paletteOutput);
            memcpy(palette, dechunker->getChunkAt(i+4), dechunker->getChunkSize());
            p->setPalette(palette);
            p->setNewPalette(newPalette);
            p->convert();
            outFilePalette.write(dechunker->getChunkAt(i+4), dechunker->getChunkSize());
            outFile.close();
            outFilePalette.close();

            this->saveAsPNG(this->filename+std::to_string(imageNumber), newPalette);
        }
        inFile.close();

    } else {
        std::cout << "Error opening  " << filename;
    }
    return 0;
}

bool ItemUnpacker::saveAsPNG(std::string outFileName, char* palette) {
    auto image = new QImage(&outData[0], 64, 4*32, QImage::Format::Format_Indexed8, nullptr, nullptr);
    for (size_t i = 0; i<this->dechunker->getChunkSize(); i += 3) {
        this->colors.push_back((new QColor(palette[i], palette[i+1], palette[i+2]))->rgb());
    }

    image->setColorTable(this->colors);
    return image->save(QString::fromStdString(outFileName+".png"), "PNG");
}