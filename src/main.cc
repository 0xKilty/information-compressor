#include <bits/stdc++.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

#include "bitarray.h"
#include "encoding.h"
#include "huffman.h"

/*
TODO

*/

using namespace std;

uint32_t getFileSize(ifstream& file) {
    file.seekg(0, ios::end);
    uint32_t fileSize = file.tellg();
    file.seekg(0, ios::beg);
    return fileSize;
}

void writeOutClear(BitArray& bitArray, ofstream& outputFile) {
    bitArray.writeOut(outputFile);
    bitArray.clear();
}

void huffmanTreeTrav(bool goRight, ByteNode*& current) {
    if (goRight)
        current = current->right;
    else
        current = current->left;
}

void byteTreeTraversalWrite(char entry, int startingPos, ByteNode*& current, ByteNode* root, BitArray& bitArray, ofstream& outputFile, int& originalSize) {
    for (int i = startingPos; i >= 0; i--) {
        huffmanTreeTrav(((entry & (1 << i)) != 0), current);
        if (current->byte != '\0') {
            if (bitArray.writeByte(current->byte)) {
                writeOutClear(bitArray, outputFile);
            }
            originalSize--;
            if (originalSize == 0) {
                break;
            }
            current = root;
        }
    }
}

int getOriginalSize(ifstream &inputFile) {
    unsigned char unchar;
    char entry;
    int originalSize = 0;
    for (int i = 0; i < 4; i++) {
        inputFile.get(entry);
        unchar = static_cast<unsigned char>(entry);
        originalSize |= ((unchar) << (8 * i));
    }
    return originalSize;
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

    uint32_t inputFileSize = getFileSize(inputFile);

    auto start_time = chrono::steady_clock::now();

    if (compress) {
        cout << "Compressing " << inputFileName << " into " << outputFileName << '\n';

        unordered_map<char, int> byteFrequencies = propogateByteFrequencies(inputFile);
        priority_queue<pair<ByteNode*, int>, vector<pair<ByteNode*, int>>, CompareByteNodes> orderedFrequencies;

        double entropy = 0;
        for (const auto& pair : byteFrequencies) {
            std::pair<ByteNode*, int> byteFreq = make_pair(new ByteNode(pair.first), pair.second);
            orderedFrequencies.push(byteFreq);
            double symbolProbability = static_cast<double>(pair.second) / inputFileSize;
            entropy -= symbolProbability * log2(symbolProbability);
        }
        cout << "Entropy:           " << entropy << '\n';

        root = buildHuffmanTree(orderedFrequencies);

        unordered_map<char, pair<int, int>> codes;

        encode(root, make_pair(0, 0), codes);
        double averageCodeLength = 0;
        for (const auto& pair : codes) {
            averageCodeLength += (pair.second.second) / static_cast<double>(codes.size());
        }
        cout << "Avg. Code Length:  " << averageCodeLength << '\n';
        writePostOrderTable(bitArray, root);

        inputFile.clear();
        inputFile.seekg(0, ios::beg);

        // Write the original size of the file
        outputFile.write(reinterpret_cast<const char*>(&inputFileSize), 4);

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
        char entry;

        int originalSize = getOriginalSize(inputFile);

        ByteNode* root;
        int startPos;
        tie(root, startPos) = createHuffmanTree(inputFile);

        ByteNode* current = root;

        inputFile.seekg(inputFile.tellg() - static_cast<std::streamoff>(1));  // go back one byte

        inputFile.get(entry);
        byteTreeTraversalWrite(entry, startPos, current, root, bitArray, outputFile, originalSize);

        while (originalSize > 0) {
            inputFile.get(entry);
            byteTreeTraversalWrite(entry, 7, current, root, bitArray, outputFile, originalSize);
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