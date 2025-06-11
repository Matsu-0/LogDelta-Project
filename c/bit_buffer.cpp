#include "bit_buffer.hpp"
#include <fstream>
#include <lzma.h>
#include <zlib.h>
#include <zstd.h>
#include <iostream>

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

void BitOutBuffer::clear() {
    byte_stream.clear();
    current_bits = 0;
    bit_count = 0;
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

    if (!compression_success) {
        std::cerr << "Error: Compression failed" << std::endl;
        return false;
    }

    // Set file open mode based on mode parameter
    std::ios_base::openmode open_mode = std::ios::binary;
    if (mode == "wb") {
        open_mode |= std::ios::trunc;  // Overwrite mode
    } else {
        open_mode |= std::ios::app;    // Default to append mode
    }

    std::ofstream file(file_path, open_mode);
    if (!file) {
        std::cerr << "Error: Failed to open file '" << file_path << "' for " 
                  << (mode == "wb" ? "writing" : "appending") << std::endl;
        return false;
    }

    file.write(reinterpret_cast<const char*>(data_to_write), size_to_write);
    if (!file.good()) {
        std::cerr << "Error: Failed to write data to file '" << file_path << "'" << std::endl;
        return false;
    }

    // Clear the buffer after successful write using the clear() interface
    clear();

    return true;
}


// BitInBuffer implementation
BitInBuffer::BitInBuffer() 
    : current_bits(0)
    , bit_count(0) {
    byte_stream.reserve(1024);  // Initial capacity
}

bool BitInBuffer::read(const std::string& file_path) {
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

    // Determine compressor type based on file extension
    CompressorType compressor = CompressorType::NONE;
    std::string ext = file_path.substr(file_path.find_last_of(".") + 1);
    if (ext == "lzma") {
        compressor = CompressorType::LZMA;
    } else if (ext == "gzip") {
        compressor = CompressorType::GZIP;
    } else if (ext == "zstd") {
        compressor = CompressorType::ZSTD;
    } else if (ext == "bin") {
        compressor = CompressorType::NONE;
    } else {
        // If no recognized extension, assume no compression
        compressor = CompressorType::NONE;
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

bool BitInBuffer::decompress_lzma(std::vector<uint8_t>& output) const {
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    ret = lzma_auto_decoder(&strm, UINT64_MAX, 0);
    if (ret != LZMA_OK) return false;

    // Start with output buffer same size as input
    output.resize(byte_stream.size() * 2);

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

bool BitCompressor::compress_file(const std::string& input_path, const std::string& output_path, CompressorType compressor) {
    // Read input file
    std::ifstream in_file(input_path, std::ios::binary | std::ios::ate);
    if (!in_file) return false;

    // Get file size
    std::streamsize size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    // Read file into buffer
    std::vector<uint8_t> input_data(size);
    if (!in_file.read(reinterpret_cast<char*>(input_data.data()), size)) {
        return false;
    }

    // Add appropriate extension based on compressor type
    std::string final_output_path = output_path;
    switch(compressor) {
        case CompressorType::LZMA:
            final_output_path += ".lzma";
            break;
        case CompressorType::GZIP:
            final_output_path += ".gzip";
            break;
        case CompressorType::ZSTD:
            final_output_path += ".zstd";
            break;
        case CompressorType::NONE:
            final_output_path += ".bin";
            break;
    }

    // Compress based on compressor type
    std::vector<uint8_t> compressed_data;

    switch(compressor) {
        case CompressorType::LZMA: {
            lzma_stream strm = LZMA_STREAM_INIT;
            lzma_ret ret;

            ret = lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC64);
            if (ret != LZMA_OK) return false;

            compressed_data.resize(input_data.size() + (input_data.size() / 2) + 1024);

            strm.next_in = input_data.data();
            strm.avail_in = input_data.size();
            strm.next_out = compressed_data.data();
            strm.avail_out = compressed_data.size();

            ret = lzma_code(&strm, LZMA_FINISH);
            if (ret != LZMA_STREAM_END) {
                lzma_end(&strm);
                return false;
            }

            compressed_data.resize(strm.total_out);
            lzma_end(&strm);
            break;
        }

        case CompressorType::GZIP: {
            uLong out_size = compressBound(input_data.size());
            compressed_data.resize(out_size);

            if (compress2(compressed_data.data(), &out_size, 
                         input_data.data(), input_data.size(), 
                         Z_BEST_COMPRESSION) != Z_OK) {
                return false;
            }

            compressed_data.resize(out_size);
            break;
        }

        case CompressorType::ZSTD: {
            size_t out_size = ZSTD_compressBound(input_data.size());
            compressed_data.resize(out_size);

            size_t compressed_size = ZSTD_compress(compressed_data.data(), out_size,
                                                 input_data.data(), input_data.size(),
                                                 ZSTD_maxCLevel());
            if (ZSTD_isError(compressed_size)) {
                return false;
            }

            compressed_data.resize(compressed_size);
            break;
        }

        case CompressorType::NONE:
            compressed_data = input_data;
            break;
    }

    // Write compressed data to output file
    std::ofstream out_file(final_output_path, std::ios::binary);
    if (!out_file) return false;

    out_file.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());
    if (!out_file.good()) return false;

    // Close files before deleting
    in_file.close();
    out_file.close();

    // Delete source file if compression was successful
    if (std::remove(input_path.c_str()) != 0) {
        return false;
    }

    return true;
}

#ifdef BITBUFFER_TEST
#include <iostream>
#include <random>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iomanip>

void test_bitbuffer_workflow() {
    // Test data: a mix of numbers and strings
    std::vector<uint32_t> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::string text = "Hello, World!";
    
    std::cout << "Original data:" << std::endl;
    std::cout << "Numbers: ";
    for (size_t i = 0; i < numbers.size(); ++i) {
        std::cout << numbers[i] << (i < numbers.size() - 1 ? ", " : "");
    }
    std::cout << std::endl;
    std::cout << "Text: " << text << std::endl << std::endl;
    
    // Step 1: Encode data using BitOutBuffer
    BitOutBuffer outbuf;
    std::string intermediate_file = "test_intermediate.bin";
    
    // First encode the length of numbers array (32 bits)
    outbuf.encode(numbers.size(), 32);
    
    // Then encode each number (8 bits each)
    for (uint32_t num : numbers) {
        outbuf.encode(num, 8);
    }

    outbuf.write(intermediate_file, "wb", CompressorType::NONE);
    
    // Then encode the length of text (32 bits)
    outbuf.encode(text.length(), 32);
    
    // Then encode each character (8 bits each)
    for (char c : text) {
        outbuf.encode(static_cast<uint8_t>(c), 8);
    }
    
    // Write to intermediate file
    if (!outbuf.write(intermediate_file, "ab", CompressorType::NONE)) {
        std::cout << "Failed to write intermediate file" << std::endl;
        return;
    }
    
    // Step 2: Compress the intermediate file using different compressors
    const char* compressor_names[] = {"NONE", "LZMA", "GZIP", "ZSTD"};
    CompressorType compressors[] = {CompressorType::NONE, CompressorType::LZMA, CompressorType::GZIP, CompressorType::ZSTD};
    
    for (int i = 0; i < 4; ++i) {
        std::cout << "\nTesting " << compressor_names[i] << " compression:" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        std::string compressed_file = std::string("test_compressed_") + compressor_names[i];
        if (!BitCompressor::compress_file(intermediate_file, compressed_file, compressors[i])) {
            std::cout << "Compression failed for " << compressor_names[i] << std::endl;
            continue;
        }
        
        // Step 3: Read and decode using BitInBuffer
        BitInBuffer inbuf;
        if (!inbuf.read(compressed_file + (compressors[i] == CompressorType::NONE ? ".bin" : 
                                        compressors[i] == CompressorType::LZMA ? ".lzma" :
                                        compressors[i] == CompressorType::GZIP ? ".gzip" : ".zstd"))) {
            std::cout << "Failed to read compressed file for " << compressor_names[i] << std::endl;
            continue;
        }
        
        // Decode numbers array
        uint32_t num_count = inbuf.decode(32);
        std::cout << "Number count: " << num_count << " (expected: " << numbers.size() << ")" << std::endl;
        
        if (num_count != numbers.size()) {
            std::cout << "Number count mismatch!" << std::endl;
            continue;
        }
        
        std::vector<uint32_t> decoded_numbers;
        std::cout << "Decoded numbers:" << std::endl;
        for (uint32_t j = 0; j < num_count; ++j) {
            uint32_t decoded = inbuf.decode(8);
            decoded_numbers.push_back(decoded);
            std::cout << "  Number[" << j << "]: " << decoded << " (expected: " << numbers[j] << ")" 
                      << (decoded == numbers[j] ? " ✓" : " ✗") << std::endl;
        }
        
        // Decode text
        uint32_t text_length = inbuf.decode(32);
        std::cout << "\nText length: " << text_length << " (expected: " << text.length() << ")" << std::endl;
        
        if (text_length != text.length()) {
            std::cout << "Text length mismatch!" << std::endl;
            continue;
        }
        
        std::string decoded_text;
        std::cout << "Decoded text:" << std::endl;
        for (uint32_t j = 0; j < text_length; ++j) {
            char decoded = static_cast<char>(inbuf.decode(8));
            decoded_text += decoded;
            std::cout << "  Char[" << j << "]: '" << decoded << "' (expected: '" << text[j] << "')"
                      << (decoded == text[j] ? " ✓" : " ✗") << std::endl;
        }
        
        // Verify the decoded data
        bool numbers_ok = true;
        for (size_t j = 0; j < numbers.size(); ++j) {
            if (decoded_numbers[j] != numbers[j]) {
                numbers_ok = false;
                break;
            }
        }
        
        bool text_ok = (decoded_text == text);
        
        std::cout << "\nFinal results:" << std::endl;
        std::cout << "Numbers match: " << (numbers_ok ? "Yes" : "No") << std::endl;
        std::cout << "Text matches: " << (text_ok ? "Yes" : "No") << std::endl;
        std::cout << "Overall test: " << (numbers_ok && text_ok ? "PASS" : "FAIL") << std::endl;
    }
}

int main() {
    test_bitbuffer_workflow();
    return 0;
}
#endif // BITBUFFER_TEST


