#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <queue>
#include <bits/stdc++.h>

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

void encode(Node* root, string code, map<char, string>& codes) {
    if (root == nullptr) { return; }

    if (root->byte != '\0') {
        codes[root->byte] = code;
    }

    encode(root->left, code + "0", codes);
    encode(root->right, code + "1", codes);
}



int main() {
    ifstream inputFile;
    string filename = "./data/text";

    inputFile.open(filename, ios::binary);

    if (!inputFile.is_open()) { return 1; }

    int fileSize = getFileSize(inputFile);
    unordered_map<uint8_t, int> byteFrequencies = propogateByteFrequencies(inputFile);
    priority_queue<Node*, vector<Node*>, CompareNodes> orderedFrequencies;
    double entropy = 0;

    for (const auto& pair : byteFrequencies) {
        orderedFrequencies.push(new Node(pair.first, pair.second));
        double symbolProbability = static_cast<double>(pair.second)/fileSize;
        entropy -= symbolProbability * log2(symbolProbability);
    }
    

    Node* root = buildTree(orderedFrequencies);

    map<char, string> codes;
    encode(root, "", codes);

    double deflated = 0;
    for (const auto& pair : byteFrequencies) {
        char character = pair.first;
        int frequency = pair.second;
        string binary = codes[character];
        deflated += (8 - static_cast<double>(binary.length())) * frequency;
        //cout << "'" << character << "': " << frequency << " - Code: " << binary << '\n';
    }
    
    cout << "Entropy: " << entropy << "\nOriginal: " << fileSize << "bytes\nCompressed: " << deflated/8 << "bytes\n";
    
    /*
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

    inputFile.close();
    return 0;
}
