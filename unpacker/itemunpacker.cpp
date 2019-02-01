#include "itemunpacker.h"

ItemUnpacker::ItemUnpacker(std::string filename) : Unpacker (filename)
{
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
        int imageNumber = 0;
        while (!inFile.eof()) {
            char output[255];
            sprintf(output, "%s%04d.data", filename.c_str(), imageNumber);
            outFile.open(output, std::fstream::binary);
            printf("Creating %s\n", output);
            sprintf(output, "%s%04d.data.pal", filename.c_str(), imageNumber);
            outFilePalette.open(output, std::fstream::binary);
            printf("Creating %s\n", output);
            //Reading from files
            inFile.read(buffer, BUFFER_SIZE);
            inFile.read(palette, PALETTE_SIZE);
            //converting palette to rgb888
            p->setPalette(palette);
            p->setNewPalette(newPalette);
            p->convert();
            //writing to new files
            outFile.write(buffer, BUFFER_SIZE);
            outFilePalette.write(newPalette, CONVERTED_PALETTE_SIZE);
            //closing files
            outFile.close();
            outFilePalette.close();
            imageNumber++;
        }
        std::cout << std::endl << "Number of converted images: " << imageNumber << std::endl;
        inFile.close();

    } else {
        std::cout << "Error opening  " << filename;
    }
    return 0;
}

