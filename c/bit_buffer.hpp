#ifndef BIT_BUFFER_HPP
#define BIT_BUFFER_HPP

#include <cstdint>
#include <string>
#include <vector>

// Compression types
enum class CompressorType {
    NONE,
    LZMA,
    GZIP,
    ZSTD
};

class BitOutBuffer {
public:
    // Constructor and destructor
    BitOutBuffer();
    ~BitOutBuffer() = default;

    // Delete copy constructor and assignment operator
    BitOutBuffer(const BitOutBuffer&) = delete;
    BitOutBuffer& operator=(const BitOutBuffer&) = delete;

    // Public methods
    void encode(uint32_t data, uint8_t bit_len = 8);
    void pack();
    size_t length();
    bool write(const std::string& file_path, const std::string& mode = "wb", CompressorType compressor = CompressorType::NONE);
    const std::vector<uint8_t>& get_bytes() const { return byte_stream; }
    void clear();

private:
    static constexpr uint8_t BYTE_LENGTH = 8;
    std::vector<uint8_t> byte_stream;
    uint32_t current_bits;
    uint8_t bit_count;

    // Private methods
    void flush();
    bool compress_lzma(std::vector<uint8_t>& output) const;
    bool compress_gzip(std::vector<uint8_t>& output) const;
    bool compress_zstd(std::vector<uint8_t>& output) const;
};

class BitInBuffer {
public:
    // Constructor and destructor
    BitInBuffer();
    ~BitInBuffer() = default;

    // Delete copy constructor and assignment operator
    BitInBuffer(const BitInBuffer&) = delete;
    BitInBuffer& operator=(const BitInBuffer&) = delete;

    // Public methods
    uint32_t decode(uint8_t bit_len);
    bool read(const std::string& file_path);
    bool read(const std::string& file_path, CompressorType compressor);

    bool is_aligned() const { return bit_count % 8 == 0; }
    void align() {
        while (!is_aligned()) {
            decode(1);
        }
    }

private:
    static constexpr uint8_t BYTE_LENGTH = 8;
    std::vector<uint8_t> byte_stream;
    uint32_t current_bits;  // Temporary buffer for unused bits
    uint8_t bit_count;      // Number of bits in current_bits

    // Private methods
    bool decompress_lzma(std::vector<uint8_t>& output) const;
    bool decompress_gzip(std::vector<uint8_t>& output) const;
    bool decompress_zstd(std::vector<uint8_t>& output) const;
};

class BitCompressor {
public:
    static bool compress_file(const std::string& input_path, const std::string& output_path, CompressorType compressor);
};

#endif // BIT_BUFFER_HPP
