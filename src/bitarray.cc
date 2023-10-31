#include "bitarray.h"
#include <iostream>

int BitArray::writeBits(int &bits, int &bitLength) {
    while (bitLength != 0) {
        if (capacity - bitLength >= 0) {
            buffer[index] |= bits << (capacity - bitLength);
            capacity -= bitLength;
            bitLength = 0;
        } else {
            buffer[index] |= bits >> (bitLength - capacity);
            bitLength -= capacity;
            bits &= ~((~0) << bitLength);
            index++;
            if (index == size) {
                return bitLength;
            } else {
                capacity = 8;
            }
        }
    }
    return 0;
}

void BitArray::writeOut(std::ofstream& outStream) {
    outStream.write(reinterpret_cast<char*>(buffer), index);
}

bool BitArray::writeByte(uint8_t byte) {
    buffer[index] = byte;
    index++;
    return index == size;
}

void BitArray::clear() {
    index = 0;
    capacity = 8;
    for (auto& entry : buffer) {
        entry = 0;
    }
}