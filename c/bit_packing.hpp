#ifndef BIT_PACKING_HPP
#define BIT_PACKING_HPP

#include <vector>
#include <string>

// Function declarations
std::vector<unsigned char> bit_packing_encode(const std::vector<int>& arr);
std::vector<int> bit_packing_decode(const std::vector<unsigned char>& encoded, int original_length); 


#endif // BIT_PACKING_HPP