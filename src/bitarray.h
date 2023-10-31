#ifndef BITARRAY_H
#define BITARRAY_H

#include <fstream>
#include <iostream>


struct BitArray {
    unsigned char buffer[64] = {0};
    int index = 0;
    int capacity = 8;
    size_t size = 64;

    BitArray() {}
    int writeBits(int &bits, int &bitLength);
    void writeOut(std::ofstream& outStream);
    bool writeByte(uint8_t byte);
    void clear();
};

#endif // BITARRAY_H
