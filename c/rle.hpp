#ifndef RLE_HPP
#define RLE_HPP

#include <vector>
#include <string>

struct RLEEncoded {
    std::vector<unsigned char> bytes;
    size_t interval_count;
};

// Convert array of 0s and 1s to byte stream
RLEEncoded rleEncode(const std::vector<int>& arr);

// Convert byte stream back to array of 0s and 1s
std::vector<int> rleDecode(const std::vector<unsigned char>& encoded, size_t interval_count);

#endif // RLE_HPP 