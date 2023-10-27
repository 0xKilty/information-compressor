/* TODO

Write the compressed char array to a file
Read in from a compressed file
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

int main() {
    ifstream inputFile;
    string filename = "./data/lorem";

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

    unordered_map<char, pair<int, int>> codes;
    encode(root, make_pair(0, 0), codes);

    map<char, string> stringCodes;
    encodeString(root, "", stringCodes);

    for (const auto& entry : codes) {
        bitset<16> x(entry.second.first);
        cout << entry.first << ' ' << x << ' ' << entry.second.second << '\n';
    }

    cout << "Entropy: " << entropy << '\n';

    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    char byte;
    unsigned char dataBuffer[64] = {0};

    cout << "\n\n";
    int byteCapacity = 8;
    int dataBufferIndex = 0;
    int limit = 0;
    while (inputFile.get(byte)) {
        if (limit == 20) {
            break;
        }
        limit++;
        int codeLength = codes[byte].second;
        int code = codes[byte].first;
        cout << stringCodes[byte];
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
                byteCapacity = 8;
            }
            if (byteCapacity - codeLength == 0) {
                dataBufferIndex++;
                byteCapacity = 8;
            }

        }
    }

    cout << '\n';
    for (int i = 0; i < 10; i++) {
        bitset<8> x(dataBuffer[i]);
        cout << x;
    }
    cout << "\n\n";
    Node* current = root;
    for (const auto &entry : dataBuffer) {
        for (int i = 7; i >= 0; i--) {
            if ((entry & (1 << i)) != 0) {
                current = current->right;
            } else {
                current = current->left;
            }
            if (current->byte != '\0') {
                cout << current->byte;
                current = root;
            }
        }
    }

    inputFile.close();
    return 0;
}

/*
    double deflated = 0;
    for (const auto& pair : byteFrequencies) {
        char character = pair.first;
        int frequency = pair.second;
        string binary = codes[character];
        deflated += (8 - static_cast<double>(binary.length())) * frequency;
        cout << static_cast<int>(character) << ": " << frequency << " - Code: " << binary << '\n';
    }

    const int numBytesToRead = 100;
    char buffer[numBytesToRead];

    while (!inputFile.eof()) {
        inputFile.read(buffer, numBytesToRead);
        if (inputFile) {
            for (int i = 0; i < inputFile.gcount(); ++i) {
                cout << static_cast<int>(buffer[i]) << "\n";
            }
        } else {
            if (inputFile.eof()) {
                cout << "End of file reached.\n";
            } else if (inputFile.fail()) {
                cerr << "Error reading from the file.\n";
            }
        }
    }
    */
