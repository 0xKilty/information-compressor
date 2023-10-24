#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <bits/stdc++.h> 

int main() {
    std::ifstream inputFile;
    std::string filename = "./data/text";

    inputFile.open(filename, std::ios::binary);

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the file: " << filename << std::endl;
        return 1;
    }

    inputFile.seekg(0, std::ios::end);
    int fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    std::map<uint8_t, int> byteFrequency;

    int total = 0;
    char byte;
    while (inputFile.get(byte)) {
        byteFrequency[static_cast<uint8_t>(byte)]++;
        total++;
    }

    double entropy = 0;
    for (const auto& entry : byteFrequency) {
        double prob = static_cast<double>(entry.second)/fileSize;
        entropy += prob * std::log2(prob);
    }

    std::cout << "Entropy: " << -entropy << '\n';

    /*
    const int numBytesToRead = 100;
    char buffer[numBytesToRead];

    while (!inputFile.eof()) {
        inputFile.read(buffer, numBytesToRead);
        if (inputFile) {
            for (int i = 0; i < inputFile.gcount(); ++i) {
                std::cout << static_cast<int>(buffer[i]) << "\n";
            }
        } else {
            if (inputFile.eof()) {
                std::cout << "End of file reached.\n";
            } else if (inputFile.fail()) {
                std::cerr << "Error reading from the file.\n";
            }
        }
    }
    */

    inputFile.close();
    return 0;
}
