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
#include <filesystem>

#include "bitarray.h"
#include "huffman.h"
#include "encoding.h"

using namespace std;

int getFileSize(ifstream &file) {
    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(0, ios::beg);
    return fileSize;
}

string postTreeTraversalTable(ByteNode *node) {
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

// entry current starting-iterator bitArray
void something(char entry, ByteNode* current, ByteNode* root, BitArray bitArray, ofstream &outputFile, int startingPos) {
    for (int i = startingPos; i >= 0; i--) {
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

int usage(char filename[]) {
    cerr << filename << " Usage: \n-c <file> (file to compress)\n-d <file> (file to decompress)\n-o <output file>\n";
    return 1;
}

int main(int argc, char* argv[]) {
    int option;
    bool compress;
    string file;
    string outputFileName;
    if (argc == 1) { return usage(argv[0]); }
    while ((option = getopt(argc, argv, "c:d:o:")) != -1) {
        switch (option) {
            case 'c':
                compress = true;
                file = optarg;
                break;
            case 'd':
                compress = false;
                file = optarg;
                break;
            case 'o':
                outputFileName = optarg;
                break;
            default:
                return usage(argv[0]);
        }
    }

    if (!filesystem::exists(file)) {
        cerr << argv[0] << ": Error - " << file << " does not exist";
        return 1;
    }

    if (outputFileName == "") {
        outputFileName = file + ".dat";
    }

    ifstream inputFile(file, ios::binary);
    if (!inputFile.is_open()) {
        cerr << argv[0] << " Error - could not open " << file;
        return 1;
    }

    ofstream outputFile(outputFileName);
    BitArray bitArray;
    ByteNode* root;

    if (compress) {
        cout << "Compressing " << file << " into " << outputFileName << '\n';

        int fileSize = getFileSize(inputFile);
        unordered_map<char, int> byteFrequencies = propogateByteFrequencies(inputFile);
        priority_queue<std::pair<ByteNode*, int>, vector<std::pair<ByteNode*, int>>, CompareByteNodes> orderedFrequencies;

        double entropy = 0;
        for (const auto &pair : byteFrequencies) {
            std::pair<ByteNode*, int> byteFreq = make_pair(new ByteNode(pair.first), pair.second);
            orderedFrequencies.push(byteFreq);
            double symbolProbability = static_cast<double>(pair.second) / fileSize;
            entropy -= symbolProbability * log2(symbolProbability);
        }
        cout << "Entropy: " << entropy << '\n';

        root = buildHuffmanTree(orderedFrequencies);

        unordered_map<char, pair<int, int>> codes;
        encode(root, make_pair(0, 0), codes);
        writePostOrderTable(bitArray, root);

        createHuffmanTree(bitArray);

        inputFile.clear();
        inputFile.seekg(0, ios::beg);
        char byte;

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
    } else {
        cout << "Decompressing " << file << " into " << outputFileName << '\n';
        pair<ByteNode*, int> root = createHuffmanTree(inputFile);
        inputFile.seekg(inputFile.tellg() - static_cast<std::streamoff>(1));
        ByteNode *current = root.first;
        char entry;
        inputFile.get(entry);

        for (int i = root.second; i >= 0; i--) {
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
                current = root.first;
            }
        }
        
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
                    current = root.first;
                }
            }
        }

        // Write the rest of writeBits
        bitArray.writeOut(outputFile);
    }

    inputFile.close();
    outputFile.close();
    
    return 0;
}