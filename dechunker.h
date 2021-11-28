#ifndef DECHUNKER_H
#define DECHUNKER_H

#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>

class Dechunker {
public:
    Dechunker(std::string filename);
    ~Dechunker();

    void dechunk();
    void saveChunksToDisk();
    size_t getNumberOfChunks();
    unsigned int getChunkSize();
    char* getChunkAt(size_t index);

private:
    std::string filename;
    unsigned int chunkSize = 64*32;
    char *buffer;
    std::vector<char*> chunkVector;
    uint numberOfChunks;
    std::ifstream inFile;
    std::ofstream outFile;
    bool isDechunked = false;
};

#endif // DECHUNKER_H
