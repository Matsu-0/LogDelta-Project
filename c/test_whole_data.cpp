#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <filesystem>
#include "record_compress.hpp"
#include "utils.hpp"

// Define dataset names
const std::vector<std::string> datasets = {"Android", "Apache", "Mac", "OpenStack", "Spark", "Zookeeper", "SSH", "Linux", "Proxifier", "Thunderbird"};
// const std::vector<std::string> datasets = {"Android", "Linux", "SSH"};

// Define dataset-specific thresholds
const std::map<std::string, double> dataset_thresholds = {
    {"Android", 0.10},
    {"Apache", 0.02},
    {"Linux", 0.06},
    {"Mac", 0.02},
    {"OpenStack", 0.02},
    {"Proxifier", 0.02},
    {"Spark", 0.02},
    {"Thunderbird", 0.12},
    {"Zookeeper", 0.26},
    {"SSH", 0.18}
};

void cleanup_resources(std::map<std::string, std::vector<double>>& time_sets) {
    for (auto& pair : time_sets) {
        std::vector<double>().swap(pair.second);
    }
    std::map<std::string, std::vector<double>>().swap(time_sets);
}

void process_datasets(const std::vector<std::string>& datasets, std::map<std::string, std::vector<double>>& time_sets) {
    std::string input_dir = "../../datasets/test_dataset/";
    std::string output_dir = "../result_approx/test_dataset/";
    if (!ensure_directory_exists(output_dir)) {
        std::cerr << "Failed to create output directory: " << output_dir << std::endl;
        return;
    }
    for (const auto& dataset : datasets) {
        std::string input_file = input_dir + dataset + ".log";
        std::string output_file = output_dir + dataset;
        double threshold = 0.02;
        if (dataset_thresholds.find(dataset) != dataset_thresholds.end()) {
            threshold = dataset_thresholds.at(dataset);
        }
        if (!std::ifstream(input_file)) {
            std::cerr << "Input file does not exist: " << input_file << std::endl;
            continue;
        }
        std::cout << "Processing file: " << input_file << " with threshold: " << threshold << std::endl;
        double time = main_encoding_compress(input_file, output_file,
                                           DefaultParams::WINDOW_SIZE,
                                           threshold,
                                           DefaultParams::BLOCK_SIZE,
                                           DefaultParams::COMPRESSOR,
                                           DefaultParams::DISTANCE,
                                           DefaultParams::USE_APPROX);
        time_sets[dataset] = {time};
        std::cout << "File time cost: " << time << " seconds" << std::endl;
    }
}

void approx_encoding() {
    std::map<std::string, std::vector<double>> time_sets;
    process_datasets(datasets, time_sets);
    // Write results to CSV file
    std::string csv_path = "../result_approx/test_dataset/time_cost.csv";
    std::vector<std::string> columns = datasets;
    std::vector<std::string> first_column = {"Time"};
    std::cout << "\nWriting results to CSV..." << std::endl;
    bool write_success = write_csv(csv_path, time_sets, columns, first_column);
    if (!write_success) {
        std::cerr << "Failed to write CSV file to: " << csv_path << std::endl;
    } else {
        std::cout << "CSV file written successfully to: " << csv_path << std::endl;
    }
    cleanup_resources(time_sets);
}

int main() {
    try {
        std::cout << "\n=== Starting Approximate Encoding ===\n" << std::endl;
        approx_encoding();
        std::cout << "\n=== Approximate Encoding Completed ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}
