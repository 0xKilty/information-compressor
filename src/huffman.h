#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <queue>
#include <unordered_map>

struct ByteNode {
    char byte;
    ByteNode *left;
    ByteNode *right;

    ByteNode(char byte) : byte(byte), left(nullptr), right(nullptr) {}
};

struct CompareByteNodes {
    bool operator()(std::pair<ByteNode*, int> left, std::pair<ByteNode*, int> right);
};

ByteNode* buildHuffmanTree(std::priority_queue<std::pair<ByteNode*, int>, std::vector<std::pair<ByteNode*, int>>, CompareByteNodes> &priorityQueue);
void encode(ByteNode *root, std::pair<int, int> code, std::unordered_map<char, std::pair<int, int>> &codes);

#endif // HUFFMAN_H
