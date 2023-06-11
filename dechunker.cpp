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
    if (inFile.is_open() && !this->isDechunked){
        std::cout << "[DEBUG] Beginnig dechunking process" << '\n';
        const std::filesystem::path path = filename;
        auto size = std::filesystem::file_size(path);
        numberOfChunks = size/chunkSize;
        std::cout << "[DEBUG] Chunks to extract: " << numberOfChunks << '\n';
        int chunkNo = 0;
        while (inFile.read(buffer, this->chunkSize)) {
            chunkVector.push_back(buffer);
            buffer = new char [chunkSize];
            chunkNo++;
        }
        std::cout << "[DEBUG] Finished dechunking process" << '\n';
        this->isDechunked = true;
    }
}

void Dechunker::saveChunksToDisk() {
    if (!this->isDechunked) {
        this->dechunk();
    }
    
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
