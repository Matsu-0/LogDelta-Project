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

class BitBuffer {
public:

    // Constructor and destructor
    BitBuffer();
    ~BitBuffer() = default;

    // Delete copy constructor and assignment operator
    BitBuffer(const BitBuffer&) = delete;
    BitBuffer& operator=(const BitBuffer&) = delete;

    // Public methods
    void encode(uint32_t data, uint8_t bit_len = 8);
    void pack();
    size_t length();
    bool write(const std::string& file_path, const std::string& mode = "wb", CompressorType compressor = CompressorType::NONE);
    bool read(const std::string& file_path);
    bool read(const std::string& file_path, CompressorType compressor);

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
    bool decompress_lzma(std::vector<uint8_t>& output) const;
};

#endif // BIT_BUFFER_HPP
