#include <iostream>
#include <fstream>

#define BUFFER_SIZE 64*32*4
#define BREAK_SIZE 64*24
#define PALETTE_SIZE 64*8+BREAK_SIZE
#define FILENAME "ITEM.DAT"

using namespace std;

int main(){
    ifstream inFile;
    ofstream outFile;
    ofstream outFilePalette;
    char buffer[BUFFER_SIZE];
    char palette[PALETTE_SIZE];
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
            inFile.read(buffer, BUFFER_SIZE);
            inFile.read(palette, PALETTE_SIZE);
            outFile.write(buffer, BUFFER_SIZE);
            outFilePalette.write(palette, PALETTE_SIZE);
            imageNumber++;
            outFile.close();
            outFilePalette.close();
//            if (first){
//                first = false;
//            }
        }
        cout << "THE END" << endl;
        inFile.close();

    } else {
        cout << "Error opening file " << FILENAME;
    }
    return 0;
    }
