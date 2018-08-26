#include <iostream>
#include <fstream>

#define BUFFER_SIZE 64*32*4
#define BREAK_SIZE 64*24
#define PALETTE_SIZE 64*8+BREAK_SIZE
#define CONVERTED_PALETTE_SIZE PALETTE_SIZE/2*3
#define FILENAME "ITEM.DAT"

using namespace std;

void convertPalette(char *palette, char *newPalette){
    int newI = 0;
    uint8_t red5, green5, blue5, green25;
    uint8_t red8, green8, blue8;
    for (int i=0; i<PALETTE_SIZE; i+=2) {
        char byte1 = palette[i];
        char byte2 = palette[i+1];
        red5 = byte1 & 0b00011111;
        blue5 = byte2 & 0b01111100;
        blue5 >>= 2;
        green5 = byte2 & 0b00000011;
        green5 <<= 3;
        green25 = byte1 & 0b11100000;
        green25 >>= 5;
        green5 = green5 | green25;
        red8 = ((red5 * 527) + 23) >> 6;
        green8 = ((green5 * 527) + 23) >> 6;
        blue8 = ((blue5 * 527) + 23) >> 6;

        newPalette[newI] = red8;
        newPalette[newI+1] = green8;
        newPalette[newI+2] = blue8;
        newI += 3;
    }
    return;
}

int main(){
    ifstream inFile;
    ofstream outFile;
    ofstream outFilePalette;
    char buffer[BUFFER_SIZE];
    char palette[PALETTE_SIZE];
    char newPalette[CONVERTED_PALETTE_SIZE];

    inFile.open(FILENAME, std::fstream::binary);
    if (inFile.is_open()) {
        cout << "File opened" << endl;
        bool first = true;
        int imageNumber = 0;
        while (!inFile.eof()) {
            char output[255];
            sprintf(output, "%s%04d.data", FILENAME, imageNumber);
            outFile.open(output, std::fstream::binary);
            printf("Creating %s\n", output);
            sprintf(output, "%s%04d.data.pal", FILENAME, imageNumber);
            outFilePalette.open(output, std::fstream::binary);
            printf("Creating %s\n", output);
//            if (!first) {
//                printf("Not first");
//                inFile.read(buffer, BREAK_SIZE);
//            }
            //Reading from files
            inFile.read(buffer, BUFFER_SIZE);
            inFile.read(palette, PALETTE_SIZE);
            //converting palette to rgb888
            convertPalette(palette, newPalette);
            //writing to new files
            outFile.write(buffer, BUFFER_SIZE);
            outFilePalette.write(newPalette, CONVERTED_PALETTE_SIZE);
            //closing files
            outFile.close();
            outFilePalette.close();
//            if (first){
//                first = false;
//            }
            imageNumber++;
        }
        cout << "THE END" << endl;
        inFile.close();

    } else {
        cout << "Error opening file " << FILENAME;
    }
    return 0;
    }
