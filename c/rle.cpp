#include "rle.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>

// Calculate the number of bits in binary representation
// Return at least 2 for numbers less than 2
int getBinaryLength(int num) {
    int len = 0;
    while (num > 0) {
        len++;
        num /= 2;
    }
    return (len < 2) ? 2 : len;
}

// Encode a single number into binary string
std::string encodeNumber(int num) {

    // std::cout << num << " ";

    int n = getBinaryLength(num);
    std::string result;
    
    // Add (n-2) '1's
    result.append(n-2, '1');
    result += '0';
    
    // Convert num to binary
    std::string binary;
    if (num == 1) {
        binary = "01";
    } else {
        while (num > 0) {
            binary = (char)('0' + (num % 2)) + binary;
            num /= 2;
        }
    }
    
    result += binary;
    // std::cout << result << std::endl;
    return result;
}

// Decode a single number from binary string
int decodeNumber(const std::string& binary, size_t& pos) {
    int countOnes = 0;
    
    // Count leading ones
    while (binary[pos] == '1') {
        countOnes++;
        pos++;
    }
    
    // Skip the '0'
    pos++;
    
    // Read the binary number
    int value = 0;
    int bitsToRead = countOnes + 2;
    for (int i = 0; i < bitsToRead; i++) {
        value = (value << 1) | (binary[pos + i] - '0');
    }
    pos += bitsToRead;
    
    return value;
}

std::vector<unsigned char> rleEncode(const std::vector<int>& arr) {
    if (arr.empty()) return {};
    
    std::string result;
    result += ('0' + arr[0]);  // Store initial value
    
    // Calculate intervals between switches
    std::vector<int> intervals;
    int count = 1;
    
    for (size_t i = 1; i < arr.size(); i++) {
        if (arr[i] != arr[i-1]) {
            intervals.push_back(count);
            count = 1;
        } else {
            count++;
        }
    }
    intervals.push_back(count);
    
    // Encode each interval
    for (int interval : intervals) {
        result += encodeNumber(interval);
    }
    
    return stringToBytes(result);
}

std::vector<int> rleDecode(const std::vector<unsigned char>& encoded) {
    std::string binaryString = bytesToString(encoded);
    if (binaryString.empty()) return {};
    
    int currentValue = binaryString[0] - '0';
    size_t pos = 1;
    std::vector<int> result;
    
    while (pos < binaryString.length() && 
           (binaryString[pos] == '1' || binaryString[pos+1])) {
        int interval = decodeNumber(binaryString, pos);
        result.insert(result.end(), interval, currentValue);
        currentValue = 1 - currentValue;  // Toggle between 0 and 1
    }
    
    return result;
}

// Comment out the test main function
/*
int main() {
    std::vector<int> test = {1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0};
    
    auto encoded = rleEncode(test);
    
    std::cout << "Encoded bytes: ";
    for (unsigned char byte : encoded) {
        printf("%02X ", byte);
    }
    std::cout << "\n";
    
    auto decoded = rleDecode(encoded);
    
    std::cout << "Decoded array: ";
    for (size_t i = 0; i < test.size(); i++) {
        std::cout << "original: " << test[i] 
                 << ", decoded: " << decoded[i] << "\n";
    }
    
    return 0;
}
*/