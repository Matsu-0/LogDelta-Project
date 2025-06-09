#include "bit_buffer.hpp"
#include <fstream>
#include <lzma.h>
#include <zlib.h>
#include <zstd.h>

// Define the static constant members
constexpr uint8_t BitOutBuffer::BYTE_LENGTH;
constexpr uint8_t BitInBuffer::BYTE_LENGTH;

// BitOutBuffer implementation
BitOutBuffer::BitOutBuffer() 
    : current_bits(0)
    , bit_count(0) {
    byte_stream.reserve(1024);  // Initial capacity
}

void BitOutBuffer::flush() {
    while (bit_count >= BYTE_LENGTH) {
        byte_stream.push_back(
            static_cast<uint8_t>(current_bits >> (bit_count - BYTE_LENGTH))
        );
        current_bits &= (1 << (bit_count - BYTE_LENGTH)) - 1;
        bit_count -= BYTE_LENGTH;
    }
}

void BitOutBuffer::encode(uint32_t data, uint8_t bit_len) {
    // Mask off any extra bits
    data &= (1ULL << bit_len) - 1;  // Use 64-bit literal to handle 32-bit values
    
    // Add the new bits to the current bits
    current_bits = (current_bits << bit_len) | data;
    bit_count += bit_len;
    
    // Flush complete bytes
    while (bit_count >= BYTE_LENGTH) {
        uint8_t byte = static_cast<uint8_t>(current_bits >> (bit_count - BYTE_LENGTH));
        byte_stream.push_back(byte);
        
        // Remove the byte we just wrote
        current_bits &= (1ULL << (bit_count - BYTE_LENGTH)) - 1;
        bit_count -= BYTE_LENGTH;
    }
}

void BitOutBuffer::pack() {
    if (bit_count > 0) {
        // Shift remaining bits to the left to align with byte boundary
        uint8_t byte = static_cast<uint8_t>(current_bits << (BYTE_LENGTH - bit_count));
        byte_stream.push_back(byte);
        
        current_bits = 0;
        bit_count = 0;
    }
}

size_t BitOutBuffer::length() {
    pack();
    return byte_stream.size();
}

bool BitOutBuffer::compress_lzma(std::vector<uint8_t>& output) const {
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

bool BitOutBuffer::compress_gzip(std::vector<uint8_t>& output) const {
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

bool BitOutBuffer::compress_zstd(std::vector<uint8_t>& output) const {
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

bool BitOutBuffer::write(const std::string& file_path, const std::string& mode, CompressorType compressor) {
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

// BitInBuffer implementation
BitInBuffer::BitInBuffer() 
    : current_bits(0)
    , bit_count(0) {
    byte_stream.reserve(1024);  // Initial capacity
}

bool BitInBuffer::read(const std::string& file_path) {
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

    // Reset temporary buffer
    current_bits = 0;
    bit_count = 0;

    return true;
}

bool BitInBuffer::decompress_lzma(std::vector<uint8_t>& output) const {
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

bool BitInBuffer::decompress_gzip(std::vector<uint8_t>& output) const {
    z_stream strm = {};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = byte_stream.size();
    strm.next_in = const_cast<Bytef*>(byte_stream.data());

    if (inflateInit2(&strm, 15 + 32) != Z_OK) {
        return false;
    }

    // Start with output buffer same size as input
    output.resize(byte_stream.size() * 2);
    size_t out_pos = 0;

    do {
        strm.avail_out = output.size() - out_pos;
        strm.next_out = output.data() + out_pos;

        int ret = inflate(&strm, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            inflateEnd(&strm);
            return false;
        }

        out_pos = output.size() - strm.avail_out;

        if (ret == Z_STREAM_END) {
            output.resize(out_pos);
            inflateEnd(&strm);
            return true;
        }

        // If we need more output space
        if (strm.avail_out == 0) {
            output.resize(output.size() * 2);
        }
    } while (true);
}

bool BitInBuffer::decompress_zstd(std::vector<uint8_t>& output) const {
    // Get decompressed size
    unsigned long long const decompressed_size = ZSTD_getFrameContentSize(byte_stream.data(), byte_stream.size());
    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR || decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        return false;
    }

    output.resize(decompressed_size);
    size_t const decompressed_bytes = ZSTD_decompress(output.data(), decompressed_size,
                                                    byte_stream.data(), byte_stream.size());
    
    if (ZSTD_isError(decompressed_bytes)) {
        return false;
    }

    output.resize(decompressed_bytes);
    return true;
}

bool BitInBuffer::read(const std::string& file_path, CompressorType compressor) {
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
            byte_stream = compressed_data;
            std::vector<uint8_t> decompressed;
            decompression_success = decompress_lzma(decompressed);
            if (decompression_success) {
                byte_stream = std::move(decompressed);
            }
            break;
        }
        case CompressorType::GZIP: {
            byte_stream = compressed_data;
            std::vector<uint8_t> decompressed;
            decompression_success = decompress_gzip(decompressed);
            if (decompression_success) {
                byte_stream = std::move(decompressed);
            }
            break;
        }
        case CompressorType::ZSTD: {
            byte_stream = compressed_data;
            std::vector<uint8_t> decompressed;
            decompression_success = decompress_zstd(decompressed);
            if (decompression_success) {
                byte_stream = std::move(decompressed);
            }
            break;
        }
        case CompressorType::NONE:
            byte_stream = compressed_data;
            break;
    }

    if (!decompression_success) return false;

    // Reset temporary buffer
    current_bits = 0;
    bit_count = 0;

    return true;
}

uint32_t BitInBuffer::decode(uint8_t bit_len) {
    if (bit_len == 0 || bit_len > 32) {
        throw std::invalid_argument("Bit length must be between 1 and 32");
    }
    
    uint32_t result = 0;
    uint8_t bits_remaining = bit_len;

    while (bits_remaining > 0) {
        // If we need more bits, read from the byte stream
        if (bit_count < bits_remaining) {
            if (byte_stream.empty()) {
                throw std::runtime_error("Attempting to read past end of buffer");
            }

            // Read a new byte and add it to current_bits
            uint8_t new_byte = byte_stream.front();
            byte_stream.erase(byte_stream.begin());  // Remove the byte we just read
            current_bits = (current_bits << BYTE_LENGTH) | new_byte;
            bit_count += BYTE_LENGTH;
        }

        // Extract bits from current_bits
        uint8_t bits_to_read = std::min(bits_remaining, bit_count);
        uint32_t value = (current_bits >> (bit_count - bits_to_read)) & ((1 << bits_to_read) - 1);
        
        // Update result and remaining bits
        result = (result << bits_to_read) | value;
        bits_remaining -= bits_to_read;
        bit_count -= bits_to_read;
        
        // Clear used bits from current_bits
        current_bits &= (1 << bit_count) - 1;
    }

    return result;
}

#ifdef BITBUFFER_TEST
#include <iostream>
#include <random>
#include <cassert>
#include <cstring>

void test_bitbuffer_compressors() {
    std::vector<uint32_t> test_data = {1, 2, 3, 4, 1, 4, 2, 3, 1, 2, 3, 4};
    uint8_t bits = 3;
    const char* compressor_names[] = {"NONE", "LZMA", "GZIP", "ZSTD"};
    CompressorType compressors[] = {CompressorType::NONE, CompressorType::LZMA, CompressorType::GZIP, CompressorType::ZSTD};

    for (int i = 0; i < 4; ++i) {
        BitOutBuffer outbuf;
        for (uint32_t v : test_data) {
            outbuf.encode(v, bits);
        }
        std::string fname = std::string("bitbuffer_test_") + compressor_names[i] + ".bin";
        bool write_ok = outbuf.write(fname, "wb", compressors[i]);
        if (!write_ok) {
            std::cout << "Write failed for compressor: " << compressor_names[i] << std::endl;
            continue;
        }
        BitInBuffer inbuf;
        bool read_ok = inbuf.read(fname, compressors[i]);
        if (!read_ok) {
            std::cout << "Read failed for compressor: " << compressor_names[i] << std::endl;
            continue;
        }
        bool ok = true;
        for (size_t j = 0; j < test_data.size(); ++j) {
            uint32_t decoded = inbuf.decode(bits);
            if (decoded != test_data[j]) {
                std::cout << "Compressor " << compressor_names[i] << ": Mismatch at " << j << ": original=" << test_data[j]
                          << " decoded=" << decoded << std::endl;
                ok = false;
            }
        }
        if (ok) {
            std::cout << compressor_names[i] << " encode/decode PASS" << std::endl;
        } else {
            std::cout << compressor_names[i] << " encode/decode FAIL" << std::endl;
        }
    }
}

void test_bitbuffer_compressed_content() {
    // First encode a 16-bit number (e.g., 0xABCD)
    uint32_t header = 0xABCD;
    uint8_t header_bits = 16;
    std::vector<uint32_t> test_data = {1, 2, 3, 4, 1, 4, 2, 3, 1, 2, 3, 4};
    uint8_t data_bits = 3;
    const char* compressor_names[] = {"NONE", "LZMA", "GZIP", "ZSTD"};
    CompressorType compressors[] = {CompressorType::NONE, CompressorType::LZMA, CompressorType::GZIP, CompressorType::ZSTD};

    for (int i = 0; i < 4; ++i) {
        std::string fname = std::string("bitbuffer_test_compressed_") + compressor_names[i] + ".bin";
        
        // Step 1: Write header directly to file
        {
            BitOutBuffer header_buf;
            header_buf.encode(header, header_bits);
            if (!header_buf.write(fname, "wb", CompressorType::NONE)) {
                std::cout << "Failed to write header for compressor: " << compressor_names[i] << std::endl;
                continue;
            }
        }

        // Step 2: Write compressed content to the same file
        {
            BitOutBuffer content_buf;
            for (uint32_t v : test_data) {
                content_buf.encode(v, data_bits);
            }
            if (!content_buf.write(fname, "ab", compressors[i])) {
                std::cout << "Failed to write compressed content for compressor: " << compressor_names[i] << std::endl;
                continue;
            }
        }

        // Read back and verify
        BitInBuffer inbuf;
        if (!inbuf.read(fname, compressors[i])) {
            std::cout << "Read failed for compressor: " << compressor_names[i] << std::endl;
            continue;
        }

        // First decode the header
        uint32_t decoded_header = inbuf.decode(header_bits);
        if (decoded_header != header) {
            std::cout << "Compressor " << compressor_names[i] << ": Header mismatch: original=" << header
                      << " decoded=" << decoded_header << std::endl;
            continue;
        }

        // Then decode the compressed content
        bool ok = true;
        for (size_t j = 0; j < test_data.size(); ++j) {
            uint32_t decoded = inbuf.decode(data_bits);
            if (decoded != test_data[j]) {
                std::cout << "Compressor " << compressor_names[i] << ": Mismatch at " << j << ": original=" << test_data[j]
                          << " decoded=" << decoded << std::endl;
                ok = false;
            }
        }
        if (ok) {
            std::cout << compressor_names[i] << " compressed content PASS" << std::endl;
        } else {
            std::cout << compressor_names[i] << " compressed content FAIL" << std::endl;
        }
    }
}

int main() {
    test_bitbuffer_compressors();
    test_bitbuffer_compressed_content();
    return 0;
}
#endif // BITBUFFER_TEST

