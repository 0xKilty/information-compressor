/* TODO

Define a way to specify the bit mapping at the beginning of the file
Allow users to specify input and output files with command line arguments
Allow users to flag on verbose mode to display entropy and compression stats
Commandline help message
Turn the huffman encoding into a class in a seperate file

*/

#include <bits/stdc++.h>

#include <bitset>
#include <fstream>
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>

#include "bitarray.h"

using namespace std;

int getFileSize(ifstream& file) {
    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(0, ios::beg);
    return fileSize;
}

unordered_map<uint8_t, int> propogateByteFrequencies(ifstream& file) {
    unordered_map<uint8_t, int> byteFrequencies;
    char byte;
    while (file.get(byte)) {
        byteFrequencies[static_cast<uint8_t>(byte)]++;
    }
    return byteFrequencies;
}

struct Node {
    uint8_t byte;
    int frequency;
    Node* left;
    Node* right;

    Node(uint8_t byte, int frequency) : byte(byte), frequency(frequency), left(nullptr), right(nullptr) {}
};

struct CompareNodes {
    bool operator()(Node* left, Node* right) {
        return left->frequency > right->frequency;
    }
};

Node* buildHuffmanTree(priority_queue<Node*, vector<Node*>, CompareNodes>& priorityQueue) {
    while (priorityQueue.size() > 1) {
        Node* left = priorityQueue.top();
        priorityQueue.pop();
        Node* right = priorityQueue.top();
        priorityQueue.pop();
        Node* newNode = new Node('\0', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;

        priorityQueue.push(newNode);
    }
    return priorityQueue.top();
}

void encode(Node* root, pair<int, int> code, unordered_map<char, pair<int, int>>& codes) {
    if (root == nullptr) {
        return;
    }

    if (root->byte != '\0') {
        codes[root->byte] = code;
    }

    encode(root->left, make_pair(code.first << 1, code.second + 1), codes);
    encode(root->right, make_pair((code.first << 1) | 1, code.second + 1), codes);
}

void encodeString(Node* root, string code, map<char, string>& codes) {
    if (root == nullptr) {
        return;
    }

    if (root->byte != '\0') {
        codes[root->byte] = code;
    }

    encodeString(root->left, code + '0', codes);
    encodeString(root->right, code + '1', codes);
}

string postTreeTraversalTable(Node* node) {
    string tree;
    if (node->byte == '\0') {
        tree += postTreeTraversalTable(node->left);
        tree += postTreeTraversalTable(node->right);
        tree += '0';
    } else {
        tree += '1';
        tree += node->byte;
    }
    return tree;
}

void buildPostOrderTable(BitArray& bitArray, Node* node) {
    int bits = 0;
    int length = 1;
    if (node->byte == '\0') {
        buildPostOrderTable(bitArray, node->left);
        buildPostOrderTable(bitArray, node->right);
    } else {
        bits = node->byte | (1 << 9);
        length = 9;
    }
    bitArray.writeBits(bits, length);
}

Node* createHuffmanTree(BitArray& bitArray) {
    stack<Node*> stack;
    bool readingChar = false;
    char byte;
    for (int i = 0; i < bitArray.index + 1; i++) {
        for (int i = 7; i >= 0; i--) {
            if ((bitArray.buffer[i] & (1 << j)) != 0) {
                // Write the next 8 bits to the char
                byte |= 
                break;
            } else {
                // Check if the stack has a length of 1
                // Pop the top two and make a new node
            }
        }
    }
    /*
    if (tree[i] == '1') {
        Node* newNode = new Node(tree[i + 1], 0);
        stack.push(newNode);
    } else if (tree[i] == '0') {
        if (stack.size() == 1) {
            break;
        } else {
            Node* left = stack.top();
            stack.pop();
            Node* right = stack.top();
            stack.pop();
            Node* newNode = new Node('\0', 0);
            newNode->left = left;
            newNode->right = right;
            stack.push(newNode);
        }
    }
    */
}

int main() {
    string filename = "./data/lorem";
    ifstream inputFile(filename, ios::binary);

    if (!inputFile.is_open()) {
        return 1;
    }

    int fileSize = getFileSize(inputFile);
    unordered_map<uint8_t, int> byteFrequencies = propogateByteFrequencies(inputFile);
    priority_queue<Node*, vector<Node*>, CompareNodes> orderedFrequencies;

    double entropy = 0;
    for (const auto& pair : byteFrequencies) {
        orderedFrequencies.push(new Node(pair.first, pair.second));
        double symbolProbability = static_cast<double>(pair.second) / fileSize;
        entropy -= symbolProbability * log2(symbolProbability);
    }

    Node* root = buildHuffmanTree(orderedFrequencies);

    string tree = postTreeTraversalTable(root);
    cout << tree << '\n';

    BitArray table;
    buildPostOrderTable(table, root);

    // Create reading tree
    stack<Node*> stack;
    for (int i = 0; i < tree.length(); i++) {
        if (tree[i] == '1') {
            Node* newNode = new Node(tree[i + 1], 0);
            stack.push(newNode);
        } else if (tree[i] == '0') {
            if (stack.size() == 1) {
                break;
            } else {
                Node* left = stack.top();
                stack.pop();
                Node* right = stack.top();
                stack.pop();
                Node* newNode = new Node('\0', 0);
                newNode->left = left;
                newNode->right = right;
                stack.push(newNode);
            }
        }
    }

    Node* it = stack.top();

    unordered_map<char, pair<int, int>> codes;
    encode(root, make_pair(0, 0), codes);

    map<char, string> stringCodes;
    encodeString(root, "", stringCodes);

    for (const auto& entry : codes) {
        bitset<16> x(entry.second.first);
        bitset<8> y(entry.first);
        // cout << y << ' ' << x << ' ' << entry.second.second << '\n';
    }

    cout << "Entropy: " << entropy << '\n';

    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    char byte;

    // Compression
    ofstream outputFile(filename + ".dat");
    BitArray bitArray;
    while (inputFile.get(byte)) {
        int codeLength = codes[byte].second;
        int code = codes[byte].first;
        while (bitArray.writeBits(code, codeLength) != 0) {
            bitArray.writeOut(outputFile);
            bitArray.clear();
        }
    }

    // Write the rest of bitArray
    bitArray.index++;
    bitArray.writeOut(outputFile);
    bitArray.clear();

    // Reset
    inputFile.close();
    outputFile.close();

    // Decompression
    inputFile.open(filename + ".dat");
    outputFile.open(filename + "_decompressed");
    Node* current = root;
    char entry;
    while (inputFile.get(entry)) {
        for (int i = 7; i >= 0; i--) {
            if ((entry & (1 << i)) != 0) {
                current = current->right;
            } else {
                current = current->left;
            }
            if (current->byte != '\0') {
                if (bitArray.writeByte(current->byte)) {
                    bitArray.writeOut(outputFile);
                    bitArray.clear();
                }
                current = root;
            }
        }
    }

    // Write the rest of writeBits
    bitArray.writeOut(outputFile);

    cout << '\n';
    return 0;
}