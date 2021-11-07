#include "dechunker.h"

Dechunker::Dechunker(std::string filename) {
    this->filename = filename;
    this->inFile.open(filename, std::ios::in | std::ios::binary);
    buffer = new char[this->chunkSize];
}

Dechunker::~Dechunker() {
    for (auto chunk : chunkVector) {
        delete[] chunk;
    }
    chunkVector.clear();
    delete[] buffer;
}

void Dechunker::dechunk() {
    if (inFile.is_open()){
        std::cout << "[DEBUG] Beginnig dechunking process" << std::endl;
        const std::filesystem::path path = filename;
        auto size = std::filesystem::file_size(path);
        numberOfChunks = size/chunkSize;
        std::cout << "[DEBUG] Chunks to extract: " << numberOfChunks << std::endl;
        int chunkNo = 0;
        while (inFile.read(buffer, this->chunkSize)) {
            chunkVector.push_back(buffer);
            buffer = new char [chunkSize];
            chunkNo++;
        }
        std::cout << "[DEBUG] Finished dechunking process" << std::endl;
    }
}

void Dechunker::saveChunksToDisk() {
    int chunkNo = 0;
    if (!std::filesystem::is_directory(filename+"_chunks")){
        std::filesystem::create_directory(filename+"_chunks");
    }
    for (auto chunk : chunkVector) {
        std::string outFileName = filename+"_chunks"+"/"+std::to_string(chunkNo)+".chunk";
        outFile.open(outFileName, std::ios::binary | std::ios::out | std::ios::trunc);
        outFile.write(chunk, this->chunkSize);
        outFile.flush();
        outFile.close();
        chunkNo++;
    }

}

size_t Dechunker::getNumberOfChunks() {
    return chunkVector.size();
}

unsigned int Dechunker::getChunkSize() {
    return chunkSize;
}

char *Dechunker::getChunkAt(size_t index) {
    if (index < chunkVector.size()) {
        return chunkVector.at(index);
    } else {
        return nullptr;
    }
}
