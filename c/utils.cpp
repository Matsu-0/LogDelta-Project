#include "utils.hpp"
#include <sys/stat.h>
#include <fstream>
#include <iostream>

std::vector<unsigned char> stringToBytes(const std::string& binaryString) {
    // Calculate required bytes (rounded up to multiple of 8)
    size_t outputLength = (binaryString.length() + 7) / 8;
    std::vector<unsigned char> bytes(outputLength, 0);
    
    // Convert each bit
    for (size_t i = 0; i < binaryString.length(); i++) {
        if (binaryString[i] == '1') {
            size_t byteIndex = i / 8;
            int bitIndex = 7 - (i % 8);  // MSB first
            bytes[byteIndex] |= (1 << bitIndex);
        }
    }
    
    return bytes;
}

std::string bytesToString(const std::vector<unsigned char>& bytes) {
    std::string binaryString;
    binaryString.reserve(bytes.size() * 8);
    
    for (unsigned char byte : bytes) {
        for (int bit = 7; bit >= 0; bit--) {
            binaryString += ((byte >> bit) & 1) ? '1' : '0';
        }
    }
    
    return binaryString;
}

bool create_directory(const std::string& path) {
    #ifdef _WIN32
        return _mkdir(path.c_str()) == 0;
    #else
        return mkdir(path.c_str(), 0777) == 0;
    #endif
}

bool directory_exists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

bool ensure_directory_exists(const std::string& path) {
    size_t pos = 0;
    std::string dir;
    while ((pos = path.find('/', pos)) != std::string::npos) {
        dir = path.substr(0, pos++);
        if (dir.empty()) continue;
        if (!directory_exists(dir)) {
            if (!create_directory(dir)) {
                std::cerr << "Failed to create directory: " << dir << std::endl;
                return false;
            }
        }
    }
    if (!directory_exists(path)) {
        return create_directory(path);
    }
    return true;
}

bool write_csv(const std::string& filepath,
              const std::map<std::string, std::vector<double>>& time_sets,
              const std::vector<std::string>& column_names,
              const std::vector<std::string>& first_column_values) {
    // 确保目录存在
    std::string dir = filepath.substr(0, filepath.find_last_of('/'));
    if (!ensure_directory_exists(dir)) {
        std::cerr << "Failed to create directory for CSV file: " << dir << std::endl;
        return false;
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    
    // 写入表头
    file << "Metric";  // 第一列的表头
    for (const auto& col : column_names) {
        file << "," << col;
    }
    file << "\n";
    
    // 获取最大行数
    size_t max_rows = 0;
    for (const auto& col : column_names) {
        if (time_sets.find(col) != time_sets.end()) {
            max_rows = std::max(max_rows, time_sets.at(col).size());
        }
    }
    
    // 写入数据
    for (size_t i = 0; i < max_rows; ++i) {
        // 写入第一列（指标名称）
        file << (i < first_column_values.size() ? first_column_values[i] : "Time" + std::to_string(i+1));
        
        // 写入每个数据集的值
        for (const auto& col : column_names) {
            file << ",";
            if (time_sets.find(col) != time_sets.end() && i < time_sets.at(col).size()) {
                file << time_sets.at(col)[i];
            }
        }
        file << "\n";
    }

    file.close();
    return true;
}
