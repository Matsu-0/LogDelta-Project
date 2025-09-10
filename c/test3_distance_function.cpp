#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"
#include "distance.hpp"

// Define dataset names
const std::vector<std::string> datasets = {"Android", "Apache", "Mac", "OpenStack", "Spark", "Zookeeper", "SSH", "Linux", "Proxifier", "Thunderbird"};
// const std::vector<std::string> datasets = {"Apache", "Linux",  "Proxifier", "Zookeeper", "Thunderbird"};

// Define distance function types
const std::vector<std::pair<std::string, DistanceType>> distance_functions = {
    {"COSINE", DistanceType::COSINE},
    {"MINHASH", DistanceType::MINHASH},
    {"QGRAM", DistanceType::QGRAM}
};

void cleanup_resources(std::map<std::string, std::vector<double>>& time_sets) {
    // Clean vectors in map
    for (auto& pair : time_sets) {
        std::vector<double>().swap(pair.second);
    }
    // Clean the map itself
    std::map<std::string, std::vector<double>>().swap(time_sets);
}

void approx_encoding() {
    // Define parameter range (0.0 to 1.0, step 0.02)
    std::vector<double> parameters;
    for (double p = 0.0; p <= 1.0; p += 0.02) {
        parameters.push_back(p);
    }

    // Create result directories for each distance function
    for (const auto& [dist_name, _] : distance_functions) {
        std::string output_path = "../result_new/result_approx/test3_distance/" + dist_name + "/";
        if (!ensure_directory_exists(output_path)) {
            std::cerr << "Failed to create output directory: " << output_path << std::endl;
            return;
        }
    }

    // Test each distance function
    for (const auto& [dist_name, dist_type] : distance_functions) {
        std::cout << "\n=== Testing distance function: " << dist_name << " ===" << std::endl;
        
        std::string output_path = "../result_new/result_approx/test3_distance/" + dist_name + "/";
        std::map<std::string, std::vector<double>> time_sets;

        // Calculate total tasks
        size_t total_tasks = datasets.size() * parameters.size();
        size_t current_task = 0;

        for (const auto& d : datasets) {
            std::vector<double> time_list;
            std::string tmp_path = output_path + d;
            
            if (!ensure_directory_exists(tmp_path)) {
                std::cerr << "Failed to create directory: " << tmp_path << std::endl;
                continue;
            }

            std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;

            for (double p : parameters) {
                current_task++;
                std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                          << "Dataset: " << d << ", Distance threshold: " << p 
                          << ", Function: " << dist_name << std::endl;

                std::string input_file_name = "../datasets/test1_data_size/" + d + "_20000.txt";
                std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(p);
                
                if (!std::ifstream(input_file_name)) {
                    std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                    continue;
                }
                
                double time = main_encoding_compress(input_file_name, output_file_name, 
                                                   DefaultParams::WINDOW_SIZE,
                                                   p,
                                                   DefaultParams::BLOCK_SIZE,
                                                   DefaultParams::COMPRESSOR,
                                                   dist_type,
                                                   DefaultParams::USE_APPROX);
                time_list.push_back(time);
                
                std::cout << "Time cost: " << time << " seconds" << std::endl;
            }
            time_sets[d] = time_list;
        }

        // Write results to CSV file
        std::string csv_path = output_path + "time_cost.csv";
        std::vector<std::string> first_column;
        first_column.push_back("Distance");
        for (double p : parameters) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.2f", p);
            first_column.push_back(buffer);
        }

        bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
        
        // Clean up resources before exit
        cleanup_resources(time_sets);

        if (write_success) {
            std::cout << "\nResults for " << dist_name << " written to: " << csv_path << std::endl;
        } else {
            std::cerr << "\nFailed to write results for " << dist_name << " to: " << csv_path << std::endl;
        }
    }
}

void exact_encoding() {
    // Define parameter range (0.0 to 1.0, step 0.02)
    std::vector<double> parameters;
    for (double p = 0.0; p <= 1.0; p += 0.02) {
        parameters.push_back(p);
    }

    // Create result directories for each distance function
    for (const auto& [dist_name, _] : distance_functions) {
        std::string output_path = "../result_new/result_exact/test3_distance/" + dist_name + "/";
        if (!ensure_directory_exists(output_path)) {
            std::cerr << "Failed to create output directory: " << output_path << std::endl;
            return;
        }
    }

    // Test each distance function
    for (const auto& [dist_name, dist_type] : distance_functions) {
        std::cout << "\n=== Testing distance function: " << dist_name << " ===" << std::endl;
        
        std::string output_path = "../result_new/result_exact/test3_distance/" + dist_name + "/";
        std::map<std::string, std::vector<double>> time_sets;

        // Calculate total tasks
        size_t total_tasks = datasets.size() * parameters.size();
        size_t current_task = 0;

        for (const auto& d : datasets) {
            std::vector<double> time_list;
            std::string tmp_path = output_path + d;
            
            if (!ensure_directory_exists(tmp_path)) {
                std::cerr << "Failed to create directory: " << tmp_path << std::endl;
                continue;
            }

            std::cout << "\n=== Processing dataset: " << d << " ===" << std::endl;

            for (double p : parameters) {
                current_task++;
                std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                          << "Dataset: " << d << ", Distance threshold: " << p 
                          << ", Function: " << dist_name << std::endl;

                std::string input_file_name = "../datasets/test1_data_size/" + d + "_20000.txt";
                std::string output_file_name = output_path + d + "/" + d + "_" + std::to_string(p);
                
                if (!std::ifstream(input_file_name)) {
                    std::cerr << "Input file does not exist: " << input_file_name << std::endl;
                    continue;
                }
                
                double time = main_encoding_compress(input_file_name, output_file_name, 
                                                   DefaultParams::WINDOW_SIZE,
                                                   p,
                                                   DefaultParams::BLOCK_SIZE,
                                                   DefaultParams::COMPRESSOR,
                                                   dist_type,
                                                   false);  // Set use_approx to false
                time_list.push_back(time);
                
                std::cout << "Time cost: " << time << " seconds" << std::endl;
            }
            time_sets[d] = time_list;
        }

        // Write results to CSV file
        std::string csv_path = output_path + "time_cost.csv";
        std::vector<std::string> first_column;
        first_column.push_back("Distance");
        for (double p : parameters) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.2f", p);
            first_column.push_back(buffer);
        }

        bool write_success = write_csv(csv_path, time_sets, datasets, first_column);
        
        // Clean up resources before exit
        cleanup_resources(time_sets);

        if (write_success) {
            std::cout << "\nResults for " << dist_name << " written to: " << csv_path << std::endl;
        } else {
            std::cerr << "\nFailed to write results for " << dist_name << " to: " << csv_path << std::endl;
        }
    }
}

int main() {
    try {
        std::cout << "\n=== Starting Approximate Encoding ===\n" << std::endl;
        approx_encoding();
        std::cout << "\n=== Approximate Encoding Completed ===\n" << std::endl;
        
        // std::cout << "\n=== Starting Exact Encoding ===\n" << std::endl;
        // exact_encoding();
        
        // std::cout << "\n=== All Tasks Completed ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
