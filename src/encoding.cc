#include "encoding.h"

std::unordered_map<char, int> propogateByteFrequencies(std::ifstream &file) {
    std::unordered_map<char, int> byteFrequencies;
    char byte;
    while (file.get(byte)) {
        byteFrequencies[byte]++;
    }
    return byteFrequencies;
}

void writePostOrderTable(BitArray &bitArray, ByteNode *node) {
    int bits = 0;
    int length = 1;
    if (node->byte == '\0') {
        writePostOrderTable(bitArray, node->left);
        writePostOrderTable(bitArray, node->right);
    } else {
        bits = (node->byte | (1 << 8));
        length = 9;
    }
    bitArray.writeBits(bits, length);
}

ByteNode* createHuffmanTree(std::ifstream &file) {
    std::stack<ByteNode*> stack;
    char readByte;
    char writeByte;
    int bitsToWrite = 0;
    while(file.get(readByte)) {
        for (int j = 7; j >= 0; j--) {
            if (bitsToWrite == 0) {
                if ((readByte & (1 << j)) != 0) {
                    bitsToWrite = 8;
                    if (j != 0) {
                        writeByte |= ((readByte & (~((~0) << j))) << (8 - j));
                        bitsToWrite -= j;
                    }
                    break;
                } else {
                    if (stack.size() == 1) {
                        return stack.top();
                    } else {
                        ByteNode *right = stack.top();
                        stack.pop();
                        ByteNode *left = stack.top();
                        stack.pop();
                        ByteNode *newNode = new ByteNode('\0');
                        newNode->left = left;
                        newNode->right = right;
                        stack.push(newNode);
                    }
                }
            } else {
                writeByte |= ((readByte & ((~0) << (8 - bitsToWrite))) >> (8 - bitsToWrite));
                j -= (bitsToWrite - 1);
                ByteNode *newNode = new ByteNode(writeByte);
                stack.push(newNode);
                writeByte = 0;
                bitsToWrite = 0;
            }
        }
    }
    return nullptr;
}

ByteNode* createHuffmanTree(BitArray &bitArray) {
    std::stack<ByteNode*> stack;
    char byte;
    int bitsToWrite = 0;
    for (int i = 0; i < bitArray.index + 1; i++) {
        for (int j = 7; j >= 0; j--) {
            if (bitsToWrite == 0) {
                if ((bitArray.buffer[i] & (1 << j)) != 0) {
                    bitsToWrite = 8;
                    if (j != 0) {
                        byte |= ((bitArray.buffer[i] & (~((~0) << j))) << (8 - j));
                        bitsToWrite -= j;
                    }
                    break;
                } else {
                    if (stack.size() == 1) {
                        return stack.top();
                    } else {
                        ByteNode *right = stack.top();
                        stack.pop();
                        ByteNode *left = stack.top();
                        stack.pop();
                        ByteNode *newNode = new ByteNode('\0');
                        newNode->left = left;
                        newNode->right = right;
                        stack.push(newNode);
                    }
                }
            } else {
                byte |= ((bitArray.buffer[i] & ((~0) << (8 - bitsToWrite))) >> (8 - bitsToWrite));
                j -= (bitsToWrite - 1);
                ByteNode *newNode = new ByteNode(byte);
                stack.push(newNode);
                byte = 0;
                bitsToWrite = 0;
            }
        }
    }
    return nullptr;
}
