#include "distance.hpp"
#include <cmath>
#include <random>
#include <unordered_map>

// Initialize MinHash with predefined hash function coefficients
MinHash::MinHash(int k, int numHashes) : k_(k), numHashes_(numHashes) {
    // Use fixed seed for generating hash function coefficients
    std::mt19937_64 gen(12345);
    std::uniform_int_distribution<uint64_t> dis;
    
    a_.reserve(numHashes_);
    b_.reserve(numHashes_);
    for (int i = 0; i < numHashes_; i++) {
        a_.push_back(dis(gen));
        b_.push_back(dis(gen));
    }
}

uint64_t MinHash::hashFunction(const std::string& str, uint64_t a, uint64_t b) {
    // Use faster hash function (DJB2)
    uint64_t hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + c;
    }
    return (a * hash + b) % PRIME;
}

std::vector<uint64_t> MinHash::getSignature(const std::string& str) {
    // Check cache first
    auto it = signature_cache_.find(str);
    if (it != signature_cache_.end()) {
        return it->second;
    }

    std::vector<uint64_t> signature(numHashes_, UINT64_MAX);
    
    // Check if string is too short
    if (str.length() < k_) {
        signature_cache_[str] = signature;
        return signature;
    }

    // Use direct pointer access with bounds checking
    const char* data = str.data();
    const size_t len = str.length();
    
    try {
        for (size_t i = 0; i + k_ <= len; ++i) {
            // Calculate base hash without creating new strings
            uint64_t base_hash = 5381;
            for (size_t j = 0; j < k_; ++j) {
                base_hash = ((base_hash << 5) + base_hash) + 
                           static_cast<unsigned char>(data[i + j]);
            }
            
            // Update all hash functions in one pass
            for (int j = 0; j < numHashes_; j++) {
                uint64_t current_hash = (a_[j] * base_hash + b_[j]) % PRIME;
                signature[j] = std::min(signature[j], current_hash);
            }
        }
    } catch (const std::exception& e) {
        // If any error occurs, return the default signature
        return signature;
    }

    // Cache the result
    signature_cache_[str] = signature;
    return signature;
}

double MinHash::estimateDistance(const std::vector<uint64_t>& sig1, 
                               const std::vector<uint64_t>& sig2) {
    int matches = 0;
    for (int i = 0; i < numHashes_; i++) {
        if (sig1[i] == sig2[i]) matches++;
    }
    return 1.0 - static_cast<double>(matches) / numHashes_;
}

double Distance::minHashDistance(const std::string& str1, const std::string& str2, int k, int numHashes) {
    static MinHash& minhash = MinHash::getInstance();
    
    // If strings are identical, return 0.0 immediately
    if (str1 == str2) return 0.0;
    
    // If either string is empty or too short, return 1.0
    if (str1.empty() || str2.empty() || 
        str1.length() < k || str2.length() < k) {
        return 1.0;
    }
    
    try {
        auto sig1 = minhash.getSignature(str1);
        auto sig2 = minhash.getSignature(str2);
        return minhash.estimateDistance(sig1, sig2);
    } catch (const std::exception& e) {
        return 1.0;  // Return maximum distance on error
    }
}

std::unordered_map<std::string, int> Distance::generateQgrams(const std::string& str, int q) {
    std::unordered_map<std::string, int> qgrams;
    
    if (str.length() < q) return qgrams;
    // Generate q-grams using sliding window
    for (size_t i = 0; i <= str.length() - q; ++i) {
        std::string qgram = str.substr(i, q);
        qgrams[qgram]++;
    }
    
    return qgrams;
}

double Distance::qgramCosineDistance(const std::string& str1, const std::string& str2, int q) {
    // Generate q-gram vectors for both strings
    auto qgrams1 = generateQgrams(str1, q);
    auto qgrams2 = generateQgrams(str2, q);
    
    // Calculate dot product
    double dotProduct = 0.0;
    for (const auto& pair : qgrams1) {
        const std::string& qgram = pair.first;
        int count = pair.second;
        if (qgrams2.count(qgram) > 0) {
            dotProduct += count * qgrams2[qgram];
        }
    }
    
    // Calculate vector magnitudes
    double norm1 = 0.0;
    double norm2 = 0.0;
    
    for (const auto& pair : qgrams1) {
        norm1 += pair.second * pair.second;
    }
    
    for (const auto& pair : qgrams2) {
        norm2 += pair.second * pair.second;
    }
    
    norm1 = std::sqrt(norm1);
    norm2 = std::sqrt(norm2);
    
    // Avoid division by zero
    if (norm1 == 0.0 || norm2 == 0.0) {
        return 1.0; // Return maximum distance if either string is empty
    }
    
    // Calculate cosine similarity and convert to distance
    double similarity = dotProduct / (norm1 * norm2);
    return 1.0 - similarity; // Convert similarity to distance
}

double Distance::calculateDistance(const std::string& str1, const std::string& str2, 
                                 DistanceType distance_type, int q) {
    switch (distance_type) {
        case DistanceType::COSINE:
            return qgramCosineDistance(str1, str2, q);
        case DistanceType::MINHASH:
            return minHashDistance(str1, str2, q);
        case DistanceType::QGRAM: {
            auto [op_list, distance] = getQgramMatchOplist(str1, str2, q);
            // Use the length of the longer string as denominator
            size_t max_length = std::max(str1.length(), str2.length());
            // Return min(distance/max_length, 1.0) to ensure result is in [0,1]
            return std::min(1.0, static_cast<double>(distance) / max_length);
        }
        default:
            return 1.0;
    }
}

#ifdef DISTANCE_TEST
int main() {
    std::string str1 = "Jun  9 06:06:51 combo anacron: anacron startup succeeded";
    std::string str2 = "Jun  9 06:06:51 combo atd: atd startup succeeded";
    double distance = Distance::qgramCosineDistance(str1, str2, 3);
    std::cout << distance << std::endl;
    return 0;
}
#endif // DISTANCE_TEST