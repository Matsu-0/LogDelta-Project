#ifndef DISTANCE_HPP
#define DISTANCE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "qgram_match.hpp"

class MinHash {
public:
    static MinHash& getInstance() {
        static MinHash instance(3, 50);  // 减少哈希函数数量到50个
        return instance;
    }
    
    std::vector<uint64_t> getSignature(const std::string& str);
    double estimateDistance(const std::vector<uint64_t>& sig1, 
                          const std::vector<uint64_t>& sig2);
    void clearCache() { signature_cache_.clear(); }  // Clear cache when needed

private:
    MinHash(int k, int numHashes);
    int k_;
    int numHashes_;
    std::vector<uint64_t> a_;  // 固定的哈希函数系数
    std::vector<uint64_t> b_;
    static constexpr uint64_t PRIME = 1099511628211ULL;
    uint64_t hashFunction(const std::string& str, uint64_t a, uint64_t b);
    
    // Cache for string signatures
    std::unordered_map<std::string, std::vector<uint64_t>> signature_cache_;
};

// 添加距离函数类型的枚举
enum class DistanceType {
    COSINE,     // Q-gram cosine distance
    MINHASH,    // MinHash distance
    QGRAM       // Q-gram match distance
};

class Distance {
public:
    // Generate q-gram vector for a string
    static std::unordered_map<std::string, int> generateQgrams(const std::string& str, int q = 3);

    // Calculate distances between two strings
    static double qgramCosineDistance(const std::string& str1, const std::string& str2, int q = 3);
    static double minHashDistance(const std::string& str1, const std::string& str2, int k = 3, int numHashes = 50);
    
    // 添加通用的距离计算函数
    static double calculateDistance(const std::string& str1, const std::string& str2, 
                                  DistanceType distance_type, int q = 3);
};

#endif // DISTANCE_HPP
