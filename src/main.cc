#include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>

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

void huffmanTreeTrav(bool goRight, ByteNode* &current) {
    if (goRight)
        current = current->right;
    else
        current = current->left;
}

void byteTreeTraversalWrite(char entry, int startingPos, ByteNode* &current, ByteNode* root, BitArray &bitArray, ofstream& outputFile) {
    for (int i = startingPos; i >= 0; i--) {
        huffmanTreeTrav(((entry & (1 << i)) != 0), current);
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
            default:
                return usage(argv[0]);
        }
    }

    if (!filesystem::exists(inputFileName)) {
        cerr << argv[0] << ": Error - " << inputFileName << " does not exist\n";
        return 1;
    }

    if (outputFileName.empty())
        outputFileName = inputFileName + ".dat";

    ifstream inputFile(inputFileName, ios::binary);
    if (!inputFile.is_open()) {
        cerr << argv[0] << " Error - could not open " << inputFileName << '\n';
        return 1;
    }

    ofstream outputFile(outputFileName);
    BitArray bitArray;
    ByteNode* root;

    int inputFileSize = getFileSize(inputFile);

    auto start_time = chrono::steady_clock::now();

    if (compress) {
        cout << "Compressing " << inputFileName << " into " << outputFileName << '\n';

        unordered_map<char, int> byteFrequencies = propogateByteFrequencies(inputFile);
        priority_queue<pair<ByteNode*, int>, vector<pair<ByteNode*, int>>, CompareByteNodes> orderedFrequencies;

        double entropy = 0;
        for (const auto &pair : byteFrequencies) {
            std::pair<ByteNode*, int> byteFreq = make_pair(new ByteNode(pair.first), pair.second);
            orderedFrequencies.push(byteFreq);
            double symbolProbability = static_cast<double>(pair.second) / inputFileSize;
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

        ByteNode *root;
        int startPos;
        tie(root, startPos) = createHuffmanTree(inputFile);

        ByteNode* current = root;

        inputFile.seekg(inputFile.tellg() - static_cast<std::streamoff>(1)); // go back one byte

        char entry;

        inputFile.get(entry);
        byteTreeTraversalWrite(entry, startPos, current, root, bitArray, outputFile);
        
        while (inputFile.get(entry)) {
            byteTreeTraversalWrite(entry, 7, current, root, bitArray, outputFile);
        }
    }

    bitArray.writeOut(outputFile);

    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    inputFile.close();
    outputFile.close();

    inputFile.open(outputFileName);
    int outputFileSize = getFileSize(inputFile);
    inputFile.close();

    int longestFileName = max(inputFileName.length(), outputFileName.length()) + 2;

    cout << "Original:          " << left << setw(longestFileName) << inputFileName << ' ' << inputFileSize << " bytes\n";
    cout << "New:               " << left << setw(longestFileName) << outputFileName << ' ' << outputFileSize << " bytes\n";
    cout << "Percentage Change: " << ((outputFileSize - inputFileSize) * 100) / static_cast<double>(inputFileSize) << "%\n";
    cout << "Time Elapsed:      " << elapsed_time.count() / 1000.0 << " ms\n";

    return 0;
}