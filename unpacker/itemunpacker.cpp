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
        dechunker->dechunk();
        size_t total = dechunker->getNumberOfChunks();
        int imageNumber = 0;
        if (!std::filesystem::is_directory("ITEM_u")){
            std::filesystem::create_directory("ITEM_u");
        }
        std::filesystem::current_path("ITEM_u");
        for (size_t i = 0; i < total; i += 5, imageNumber++) {
            char output[255];
            //add check if file is opened
            sprintf(output, "%s%04d.data", filename.c_str(), imageNumber);
            outFile.open(output, std::fstream::binary);
            printf("Creating %s\n", output);
            for (size_t j = 0; j < 4; ++j) {
               outFile.write(dechunker->getChunkAt(i+j), dechunker->getChunkSize());
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
//            //making Image
//            Magick::Image img;
//            img.depth(8);
//            img.read(64, 32, "R", Magick::StorageType::CharPixel, dechunker->getChunkAt(i));
//            //read palette
//            Magick::Image paletteImage;
//            paletteImage.size("64x32");
//            paletteImage.magick("RGB");
//            img.map(paletteImage, false);
//            img.write("out.png");
        }
        inFile.close();

    } else {
        std::cout << "Error opening  " << filename;
    }
    return 0;
}

