#include <bits/stdc++.h>
#include <iostream>
#include <string>
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

void writeOutClear(BitArray& bitArray, ofstream& outputFile) {
    bitArray.writeOut(outputFile);
    bitArray.clear();
}

void huffmanTreeHop(bool goRight, ByteNode* &current) {
    if (goRight)
        current = current->right;
    else
        current = current->left;
}

void byteTreeTraversalWrite(char entry, int startingPos, ByteNode* &current, ByteNode* root, BitArray &bitArray, ofstream& outputFile) {
    for (int i = startingPos; i >= 0; i--) {
        huffmanTreeHop(((entry & (1 << i)) != 0), current);
        if (current->byte != '\0') {
            if (bitArray.writeByte(current->byte)) {
                writeOutClear(bitArray, outputFile);
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
    bool compress = true;
    bool verbose = false;
    string inputFileName;
    string outputFileName;

    if (argc == 1) 
        return usage(argv[0]);

    while ((option = getopt(argc, argv, "c:d:o:")) != -1) {
        switch (option) {
            case 'd':
                compress = false;
            case 'c':
                inputFileName = optarg;
                break;
            case 'o':
                outputFileName = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            default:
                return usage(argv[0]);
        }
    }

    if (!filesystem::exists(inputFileName)) {
        cerr << argv[0] << ": Error - " << inputFileName << " does not exist";
        return 1;
    }

    if (outputFileName == "")
        outputFileName = inputFileName + ".dat";

    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile.is_open()) {
        cerr << argv[0] << " Error - could not open " << inputFileName;
        return 1;
    }

    ofstream outputFile(outputFileName);
    BitArray bitArray;
    ByteNode* root;

    if (compress) {
        cout << "Compressing " << inputFileName << " into " << outputFileName << '\n';

        int fileSize = getFileSize(inputFile);
        unordered_map<char, int> byteFrequencies = propogateByteFrequencies(inputFile);
        priority_queue<pair<ByteNode*, int>, vector<pair<ByteNode*, int>>, CompareByteNodes> orderedFrequencies;

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

        inputFile.clear();
        inputFile.seekg(0, ios::beg);

        char byte;
        while (inputFile.get(byte)) {
            int codeLength = codes[byte].second;
            int code = codes[byte].first;
            while (bitArray.writeBits(code, codeLength) != 0)
                writeOutClear(bitArray, outputFile);
        }

        bitArray.index++;        
    } else {
        cout << "Decompressing " << inputFileName << " into " << outputFileName << '\n';

        pair<ByteNode*, int> root = createHuffmanTree(inputFile); // returns the top of the huffman tree and where it left off in the byte

        inputFile.seekg(inputFile.tellg() - static_cast<std::streamoff>(1)); // go back one byte

        ByteNode *current = root.first;
        char entry;

        inputFile.get(entry);
        byteTreeTraversalWrite(entry, root.second, current, root.first, bitArray, outputFile);
        
        while (inputFile.get(entry)) {
            byteTreeTraversalWrite(entry, 7, current, root.first, bitArray, outputFile);
        }
    }

    bitArray.writeOut(outputFile); // write out the rest of bitArray
    inputFile.close();
    outputFile.close();
    
    return 0;
}