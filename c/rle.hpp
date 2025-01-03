#ifndef RLE_HPP
#define RLE_HPP

#include <vector>
#include <string>


// Convert array of 0s and 1s to byte stream
std::vector<unsigned char> rleEncode(const std::vector<int>& arr);

// Convert byte stream back to array of 0s and 1s
std::vector<int> rleDecode(const std::vector<unsigned char>& encoded);

#endif // RLE_HPP 