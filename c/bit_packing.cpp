#include "bit_packing.hpp"
#include <stdexcept>
#include <iostream>

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
    while (binary_string.length() < total_bytes * 8) {
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

    // Convert bytes to binary string
    std::string binary_string;
    binary_string.reserve(encoded.size() * 8);
    
    for (unsigned char byte : encoded) {
        for (int j = 7; j >= 0; j--) {
            binary_string += '0' + ((byte >> j) & 1);
        }
    }

    // Read bit_width from first byte
    int bit_width = 0;
    for (int i = 0; i < 8; i++) {
        bit_width = (bit_width << 1) | (binary_string[i] - '0');
    }

    // Extract values
    std::vector<int> result(original_length);
    for (int i = 0; i < original_length; i++) {
        int val = 0;
        int start_pos = 8 + i * bit_width;
        
        for (int j = 0; j < bit_width; j++) {
            val = (val << 1) | (binary_string[start_pos + j] - '0');
        }
        result[i] = val;
    }

    return result;
}

// Comment out the test main function
/*
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
*/