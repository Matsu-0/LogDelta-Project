#include "bit_packing.hpp"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <cmath>

// Calculate the number of bits needed to represent a number
static int get_bit_width(int num) {
    int width = 0;
    while (num > 0) {
        width++;
        num >>= 1;
    }
    return width;
}

// Find maximum value in vector
static int find_max(const std::vector<int>& arr) {
    if (arr.empty()) {
        throw std::invalid_argument("Empty array");
    }
    return *std::max_element(arr.begin(), arr.end());
}

std::vector<unsigned char> bit_packing_encode(const std::vector<int>& arr) {
    if (arr.empty()) {
        return {};
    }

    // Find maximum value and calculate bit width
    int max_val = find_max(arr);
    int bit_width = get_bit_width(max_val);

    // Calculate total bits and bytes needed
    int total_bits = arr.size() * bit_width;
    int total_bytes = (total_bits + 7) / 8 + 1;  // +1 for bit_width byte

    // Create binary string
    std::string binary_string;
    binary_string.reserve(total_bytes * 8);

    // First byte: bit_width
    for (int j = 7; j >= 0; j--) {
        binary_string += '0' + ((bit_width >> j) & 1);
    }

    // Add all numbers
    for (int val : arr) {
        for (int j = bit_width - 1; j >= 0; j--) {
            binary_string += '0' + ((val >> j) & 1);
        }
    }

    // Pad with zeros
    while ( (int) binary_string.length() < total_bytes * 8) {
        binary_string += '0';
    }

    // Convert to bytes
    std::vector<unsigned char> result(total_bytes);
    for (int i = 0; i < total_bytes; i++) {
        unsigned char byte = 0;
        for (int j = 0; j < 8; j++) {
            byte = (byte << 1) | (binary_string[i * 8 + j] - '0');
        }
        result[i] = byte;
    }

    return result;
}

std::vector<int> bit_packing_decode(const std::vector<unsigned char>& encoded, int original_length) {
    if (encoded.empty() || original_length <= 0) {
        return {};
    }

    // Read bit_width directly from first byte
    int bit_width = encoded[0];
    
    // Pre-allocate result vector
    std::vector<int> result;
    result.reserve(original_length);
    
    // Extract values directly from bytes without string conversion
    // int bit_pos = 8;  // Start after bit_width byte
    int byte_pos = 1; // Start from second byte
    int bit_in_byte = 0;
    
    for (int i = 0; i < original_length; i++) {
        int val = 0;
        
        // Read bit_width bits for this value
        for (int j = 0; j < bit_width; j++) {
            if (bit_in_byte >= 8) {
                byte_pos++;
                bit_in_byte = 0;
            }
            
            int bit = (encoded[byte_pos] >> (7 - bit_in_byte)) & 1;
            val = (val << 1) | bit;
            bit_in_byte++;
        }
        
        result.push_back(val);
    }

    return result;
}

#ifdef BITPACKING_TEST
int main() {
    std::vector<int> test = {5, 7, 3, 3, 4, 2, 4, 2, 5, 12, 23};
    // Encode
    std::vector<unsigned char> encoded = bit_packing_encode(test);
    // Decode
    std::vector<int> decoded = bit_packing_decode(encoded, test.size());
    // Print results
    std::cout << "Encoded bytes: ";
    for (unsigned char byte : encoded) {
        std::cout << std::hex;
        if (static_cast<int>(byte) < 16) {
            std::cout << "0";
        }
        std::cout << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << "\n";
    std::cout << "Decoded values:\n";
    for (size_t i = 0; i < test.size(); i++) {
        std::cout << "original: " << test[i] << ", decoded: " << decoded[i] << "\n";
    }
    return 0;
}
#endif // BITPACKING_TEST