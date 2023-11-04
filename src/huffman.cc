#include "huffman.h"

bool CompareByteNodes::operator()(std::pair<ByteNode*, int> left, std::pair<ByteNode*, int> right) {
    return left.second > right.second;
}

ByteNode* buildHuffmanTree(std::priority_queue<std::pair<ByteNode*, int>, std::vector<std::pair<ByteNode*, int>>, CompareByteNodes> &priorityQueue) {
    while (priorityQueue.size() > 1) {
        
        std::pair<ByteNode*, int> left = priorityQueue.top();
        priorityQueue.pop();
        std::pair<ByteNode*, int> right = priorityQueue.top();
        priorityQueue.pop();

        ByteNode *newNode = new ByteNode('\0');
        newNode->left = left.first;
        newNode->right = right.first;
        std::pair<ByteNode*, int> parentNode = std::make_pair(newNode, left.second + right.second);
        
        priorityQueue.push(parentNode);
    }
    return priorityQueue.top().first;
}

void encode(ByteNode* root, std::pair<int, int> code, std::unordered_map<char, std::pair<int, int>> &codes) {
    if (root == nullptr) { return; }

    if (root->byte != '\0') {
        codes[root->byte] = code;
    }
    encode(root->left, std::make_pair(code.first << 1, code.second + 1), codes);
    encode(root->right, std::make_pair((code.first << 1) | 1, code.second + 1), codes);
}

