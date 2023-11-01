#ifndef ENCODING_H
#define ENCODING_H

#include <fstream>
#include <unordered_map>
#include <stack>
#include "bitarray.h"
#include "huffman.h"

std::unordered_map<char, int> propogateByteFrequencies(std::ifstream &file);
void writePostOrderTable(BitArray &bitArray, ByteNode *node);
ByteNode* createHuffmanTree(BitArray &bitArray);
ByteNode* createHuffmanTree(std::ifstream &file);

#endif // ENCODING_H