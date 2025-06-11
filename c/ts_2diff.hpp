#ifndef TS_2DIFF_HPP
#define TS_2DIFF_HPP

#include <vector>
#include <string>
#include <cstddef>
#include "bit_buffer.hpp"

size_t ts2diff_encode(const std::vector<int>& data, BitOutBuffer& stream);
size_t encode_block(BitOutBuffer& stream, const std::vector<int>& data);
std::vector<int> ts2diff_decode(BitInBuffer& stream);
std::vector<int> decode_block(BitInBuffer& stream);
std::vector<int> ts2diff_decode_from_buffer(BitInBuffer& stream);

#endif // TS_2DIFF_HPP
