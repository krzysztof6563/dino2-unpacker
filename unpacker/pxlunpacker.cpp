#include "pxlunpacker.h"

PXLUnpacker::PXLUnpacker(std::string filename) : Unpacker (filename)
{

}

int PXLUnpacker::unpack() {
    dechunker->dechunk();
    auto size = dechunker->getNumberOfChunks();
    char byte1, byte2;
    uint8_t red5, green5, blue5, green25;
    uint8_t red8, green8, blue8;
    //Creating .data filename for usage with GIMP
    char output[5000];
    sprintf(output, "%s.data", filename.c_str());
    outFile.open(output, std::fstream::binary);
    if (inFile.is_open()) {
        std::cout << "File opened" << std::endl;
        int pixelNumber = 1;
        for (size_t i=0;i<size*dechunker->getChunkSize()/2;i++) {
            //Reading 2 bytes
            inFile.read(&byte1, 1);
            inFile.read(&byte2, 1);
            //Extracting 5 red bits
            red5 = byte1 & 0b00011111;
            //Extracting 5 blue bits
            blue5 = byte2 & 0b01111100;
            blue5 >>= 2;
            //Extracting 5 green bits
            green5 = byte2 & 0b00000011;
            green5 <<= 3;
            green25 = byte1 & 0b11100000;
            green25 >>= 5;
            green5 = green5 | green25;
            //Converting to 8 bit
            red8 = ((red5 * 527) + 23) >> 6;
            green8 = ((green5 * 527) + 23) >> 6;
            blue8 = ((blue5 * 527) + 23) >> 6;
            //Writing to file
            outFile.write(reinterpret_cast<const char *>(&red8), sizeof (red8));
            outFile.write(reinterpret_cast<const char *>(&green8), sizeof (green8));
            outFile.write(reinterpret_cast<const char *>(&blue8), sizeof (blue8));
            pixelNumber++;
        }
        std::cout << "THE END" << std::endl;
        inFile.close();
        outFile.close();
    } else {
        std::cout << "Error opening file" << filename;
    }
    return 0;
}
