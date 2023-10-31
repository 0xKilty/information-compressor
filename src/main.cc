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

int getFileSize(ifstream &file) {
    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(0, ios::beg);
    return fileSize;
}

unordered_map<uint8_t, int> propogateByteFrequencies(ifstream &file) {
    unordered_map<uint8_t, int> byteFrequencies;
    char byte;
    while (file.get(byte)) {
        byteFrequencies[static_cast<uint8_t>(byte)]++;
    }
    return byteFrequencies;
}

struct ByteNode {
    uint8_t byte;
    ByteNode *left;
    ByteNode *right;
};

struct Node {
    uint8_t byte;
    int frequency;
    Node *left;
    Node *right;

    Node(uint8_t byte, int frequency)
        : byte(byte), frequency(frequency), left(nullptr), right(nullptr) {}
};

struct CompareNodes {
    bool operator()(Node *left, Node *right) {
        return left->frequency > right->frequency;
    }
};

Node *buildHuffmanTree(
    priority_queue<Node*, vector<Node*>, CompareNodes> &priorityQueue) {
    while (priorityQueue.size() > 1) {
        Node *left = priorityQueue.top();
        priorityQueue.pop();
        Node *right = priorityQueue.top();
        priorityQueue.pop();
        Node *newNode = new Node('\0', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;

        priorityQueue.push(newNode);
    }
    return priorityQueue.top();
}

void encode(Node *root, pair<int, int> code,
            unordered_map<char, pair<int, int>> &codes) {
    if (root == nullptr) {
        return;
    }
    if (root->byte != '\0') {
        codes[root->byte] = code;
    }
    encode(root->left, make_pair(code.first << 1, code.second + 1), codes);
    encode(root->right, make_pair((code.first << 1) | 1, code.second + 1),
           codes);
}

string postTreeTraversalTable(Node *node) {
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

void writePostOrderTable(BitArray &bitArray, Node *node) {
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

Node* createHuffmanTree(BitArray &bitArray) {
    stack<Node *> stack;
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
                        Node *right = stack.top();
                        stack.pop();
                        Node *left = stack.top();
                        stack.pop();
                        Node *newNode = new Node('\0', 0);
                        newNode->left = left;
                        newNode->right = right;
                        stack.push(newNode);
                    }
                }
            } else {
                byte |= ((bitArray.buffer[i] & ((~0) << (8 - bitsToWrite))) >> (8 - bitsToWrite));
                j -= (bitsToWrite - 1);
                Node *newNode = new Node(byte, 0);
                stack.push(newNode);
                byte = 0;
                bitsToWrite = 0;
            }
        }
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    int option;
    bool compress;
    string file;
    while ((option = getopt(argc, argv, "c:d:")) != -1) {
        switch (option) {
            case 'c':
                compress = true;
                file = optarg;
                break;
            case 'd':
                compress = false;
                file = optarg;
                break;
            default:
                cerr << "Usage: " << argv[0] << " -f -v value\n";
                return 1;
        }
    }

    // Check if file exists
    
    cout << file << '\n';
    if (compress) {
        cout << "Compressing " << file << '\n';
    } else {
        cout << "Decompressing " << file << '\n';
    }

    string filename = "./data/lorem";
    ifstream inputFile(filename, ios::binary);

    if (!inputFile.is_open()) {
        return 1;
    }

    int fileSize = getFileSize(inputFile);
    unordered_map<uint8_t, int> byteFrequencies = propogateByteFrequencies(inputFile);
    priority_queue<Node *, vector<Node *>, CompareNodes> orderedFrequencies;

    double entropy = 0;
    for (const auto &pair : byteFrequencies) {
        orderedFrequencies.push(new Node(pair.first, pair.second));
        double symbolProbability = static_cast<double>(pair.second) / fileSize;
        entropy -= symbolProbability * log2(symbolProbability);
    }

    Node *root = buildHuffmanTree(orderedFrequencies);

    string tree = postTreeTraversalTable(root);
    cout << tree << '\n';

    BitArray table;
    writePostOrderTable(table, root);

    Node *it = createHuffmanTree(table);

    unordered_map<char, pair<int, int>> itcodes;
    encode(root, make_pair(0, 0), itcodes);

    unordered_map<char, pair<int, int>> codes;
    encode(root, make_pair(0, 0), codes);

    for (const auto &entry : codes) {
        bitset<16> x(entry.second.first);
        bitset<16> y(itcodes[entry.first].first);
        //cout << y << ' ' << x << ' ' << entry.first << '\n';
    }

    cout << "\nEntropy: " << entropy << '\n';

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
    Node *current = it;
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
                current = it;
            }
        }
    }

    // Write the rest of writeBits
    bitArray.writeOut(outputFile);

    cout << '\n';
    return 0;
}