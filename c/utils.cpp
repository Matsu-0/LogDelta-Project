#include "utils.hpp"

std::vector<unsigned char> stringToBytes(const std::string& binaryString) {
    // Calculate required bytes (rounded up to multiple of 8)
    size_t outputLength = (binaryString.length() + 7) / 8;
    std::vector<unsigned char> bytes(outputLength, 0);
    
    // Convert each bit
    for (size_t i = 0; i < binaryString.length(); i++) {
        if (binaryString[i] == '1') {
            size_t byteIndex = i / 8;
            int bitIndex = 7 - (i % 8);  // MSB first
            bytes[byteIndex] |= (1 << bitIndex);
        }
    }
    
    return bytes;
}

std::string bytesToString(const std::vector<unsigned char>& bytes) {
    std::string binaryString;
    binaryString.reserve(bytes.size() * 8);
    
    for (unsigned char byte : bytes) {
        for (int bit = 7; bit >= 0; bit--) {
            binaryString += ((byte >> bit) & 1) ? '1' : '0';
        }
    }
    
    return binaryString;
}
