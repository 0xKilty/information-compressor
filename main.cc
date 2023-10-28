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
#include <string>
#include <unordered_map>

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

Node* buildTree(priority_queue<Node*, vector<Node*>, CompareNodes>& priorityQueue) {
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

string traversal(Node* node) {
    string tree;
    if (node->byte == '\0') {
        tree += traversal(node->left);
        tree += '0';
        tree += traversal(node->right);
    } else {
        tree += '1';
        tree += node->byte;
    }
    return tree;
}

int main() {
    string filename = "./data/lorem";
    ifstream inputFile;

    inputFile.open(filename, ios::binary);

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

    Node* root = buildTree(orderedFrequencies);

    string tree = traversal(root);
    cout << tree << '\n';

    unordered_map<char, pair<int, int>> codes;
    encode(root, make_pair(0, 0), codes);

    map<char, string> stringCodes;
    encodeString(root, "", stringCodes);

    for (const auto& entry : codes) {
        bitset<16> x(entry.second.first);
        bitset<8> y(entry.first);
        //cout << y << ' ' << x << ' ' << entry.second.second << '\n';
    }

    cout << "Entropy: " << entropy << '\n';

    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    char byte;
    const int dataBufferSize = 64;
    unsigned char dataBuffer[dataBufferSize] = {0};

    cout << "\n\n";

    // Compression
    ofstream outputFile(filename + ".dat");
    int byteCapacity = 8;
    int dataBufferIndex = 0;
    while (inputFile.get(byte)) {
        int codeLength = codes[byte].second;
        int code = codes[byte].first;
        while (codeLength != 0) {
            if (byteCapacity - codeLength >= 0) {
                dataBuffer[dataBufferIndex] |= code << (byteCapacity - codeLength);
                byteCapacity -= codeLength;
                codeLength = 0;
            } else {
                dataBuffer[dataBufferIndex] |= code >> (codeLength - byteCapacity);
                codeLength -= byteCapacity;
                code &= ~((~0) << codeLength);
                dataBufferIndex++;
                if (dataBufferIndex == dataBufferSize) {
                    outputFile.write(reinterpret_cast<char*>(dataBuffer), dataBufferSize);
                    dataBufferIndex = 0;
                    byteCapacity = 8;
                    for (auto& entry : dataBuffer) { entry = 0; }
                } else {
                    byteCapacity = 8;
                }
            }
        }
    }

    // Write the rest of dataBuffer
    outputFile.write(reinterpret_cast<char*>(dataBuffer), dataBufferIndex + 1);
    inputFile.close();
    outputFile.close();

    // Decompression
    inputFile.open(filename + ".dat");
    outputFile.open(filename + "_decompressed");
    Node* current = root;
    char entry;
    unsigned char writeBuffer[dataBufferSize] = {0};
    int writeBufferIndex = 0;
    while (inputFile.get(entry)) {
        for (int i = 7; i >= 0; i--) {
            if ((entry & (1 << i)) != 0) {
                current = current->right;
            } else {
                current = current->left;
            }
            if (current->byte != '\0') {
                writeBuffer[writeBufferIndex] = current->byte;
                writeBufferIndex++;
                if (writeBufferIndex == dataBufferSize) {
                    outputFile.write(reinterpret_cast<char*>(writeBuffer), dataBufferSize);
                    writeBufferIndex = 0;
                }
                current = root;
            }
        }
    }
    outputFile.write(reinterpret_cast<char*>(writeBuffer), writeBufferIndex);
    cout << '\n';
    return 0;
}