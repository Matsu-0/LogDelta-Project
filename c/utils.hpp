// utils.h
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

// Convert binary string to bytes
std::vector<unsigned char> stringToBytes(const std::string& binaryString);

// Convert bytes to binary string
std::string bytesToString(const std::vector<unsigned char>& bytes);

#endif // UTILS_HPP