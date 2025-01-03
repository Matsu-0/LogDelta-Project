#include "bit_buffer.hpp"
#include <fstream>
#include <lzma.h>
#include <zlib.h>
#include <zstd.h>

BitBuffer::BitBuffer() 
    : current_bits(0)
    , bit_count(0) {
    byte_stream.reserve(1024);  // Initial capacity
}

void BitBuffer::flush() {
    while (bit_count >= BYTE_LENGTH) {
        byte_stream.push_back(
            static_cast<uint8_t>(current_bits >> (bit_count - BYTE_LENGTH))
        );
        current_bits &= (1 << (bit_count - BYTE_LENGTH)) - 1;
        bit_count -= BYTE_LENGTH;
    }
}

void BitBuffer::encode(uint32_t data, uint8_t bit_len) {
    data &= (1 << bit_len) - 1;
    current_bits = (current_bits << bit_len) | data;
    bit_count += bit_len;
    flush();
}

void BitBuffer::pack() {
    flush();
    if (bit_count > 0) {
        byte_stream.push_back(
            static_cast<uint8_t>(current_bits << (BYTE_LENGTH - bit_count))
        );
        current_bits = 0;
        bit_count = 0;
    }
}

size_t BitBuffer::length() {
    pack();
    return byte_stream.size();
}

bool BitBuffer::compress_lzma(std::vector<uint8_t>& output) const {
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    ret = lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC64);
    if (ret != LZMA_OK) return false;

    output.resize(byte_stream.size() + (byte_stream.size() / 2) + 1024);

    strm.next_in = byte_stream.data();
    strm.avail_in = byte_stream.size();
    strm.next_out = output.data();
    strm.avail_out = output.size();

    ret = lzma_code(&strm, LZMA_FINISH);
    if (ret != LZMA_STREAM_END) {
        lzma_end(&strm);
        return false;
    }

    output.resize(strm.total_out);
    lzma_end(&strm);
    return true;
}

bool BitBuffer::compress_gzip(std::vector<uint8_t>& output) const {
    uLong out_size = compressBound(byte_stream.size());
    output.resize(out_size);

    if (compress2(output.data(), &out_size, 
                 byte_stream.data(), byte_stream.size(), 
                 Z_BEST_COMPRESSION) != Z_OK) {
        return false;
    }

    output.resize(out_size);
    return true;
}

bool BitBuffer::compress_zstd(std::vector<uint8_t>& output) const {
    size_t out_size = ZSTD_compressBound(byte_stream.size());
    output.resize(out_size);

    size_t compressed_size = ZSTD_compress(output.data(), out_size,
                                         byte_stream.data(), byte_stream.size(),
                                         ZSTD_maxCLevel());
    if (ZSTD_isError(compressed_size)) {
        return false;
    }

    output.resize(compressed_size);
    return true;
}

bool BitBuffer::write(const std::string& file_path, const std::string& mode, CompressorType compressor) {
    pack();

    std::vector<uint8_t> compressed_data;
    const uint8_t* data_to_write = byte_stream.data();
    size_t size_to_write = byte_stream.size();
    bool compression_success = true;

    switch(compressor) {
        case CompressorType::LZMA:
            compression_success = compress_lzma(compressed_data);
            if (compression_success) {
                data_to_write = compressed_data.data();
                size_to_write = compressed_data.size();
            }
            break;

        case CompressorType::GZIP:
            compression_success = compress_gzip(compressed_data);
            if (compression_success) {
                data_to_write = compressed_data.data();
                size_to_write = compressed_data.size();
            }
            break;

        case CompressorType::ZSTD:
            compression_success = compress_zstd(compressed_data);
            if (compression_success) {
                data_to_write = compressed_data.data();
                size_to_write = compressed_data.size();
            }
            break;

        case CompressorType::NONE:
            break;
    }

    if (!compression_success) return false;

    std::ofstream file(file_path, std::ios::binary);
    if (!file) return false;

    file.write(reinterpret_cast<const char*>(data_to_write), size_to_write);
    return file.good();
}

bool BitBuffer::read(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file) return false;

    // Get file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read file into byte_stream
    byte_stream.resize(size);
    if (!file.read(reinterpret_cast<char*>(byte_stream.data()), size)) {
        return false;
    }

    // Reset bit buffer state
    current_bits = 0;
    bit_count = 0;

    return true;
}

bool BitBuffer::decompress_lzma(std::vector<uint8_t>& output) const {
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    ret = lzma_auto_decoder(&strm, UINT64_MAX, 0);
    if (ret != LZMA_OK) return false;

    // Start with output buffer same size as input
    output.resize(byte_stream.size() * 2);
    size_t out_pos = 0;

    strm.next_in = byte_stream.data();
    strm.avail_in = byte_stream.size();
    strm.next_out = output.data();
    strm.avail_out = output.size();

    while (true) {
        ret = lzma_code(&strm, LZMA_FINISH);

        if (ret == LZMA_STREAM_END) {
            output.resize(strm.total_out);
            lzma_end(&strm);
            return true;
        }

        if (ret != LZMA_OK) {
            lzma_end(&strm);
            return false;
        }

        // If we need more output space
        if (strm.avail_out == 0) {
            size_t old_size = output.size();
            output.resize(old_size * 2);
            strm.next_out = output.data() + old_size;
            strm.avail_out = old_size;
        }
    }
}

bool BitBuffer::read(const std::string& file_path, CompressorType compressor) {
    // First read the file
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file) return false;

    // Get file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read file into temporary buffer
    std::vector<uint8_t> compressed_data(size);
    if (!file.read(reinterpret_cast<char*>(compressed_data.data()), size)) {
        return false;
    }

    // Decompress based on compressor type
    bool decompression_success = true;
    switch(compressor) {
        case CompressorType::LZMA: {
            byte_stream.clear();
            decompression_success = decompress_lzma(byte_stream);
            break;
        }
        case CompressorType::NONE:
            byte_stream = compressed_data;
            break;
        default:
            return false;  // Unsupported compression type
    }

    if (!decompression_success) return false;

    // Reset bit buffer state
    current_bits = 0;
    bit_count = 0;

    return true;
}

