#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "record_compress.hpp"
#include "utils.hpp"

// Define parameter range (0 to 20 for disturbing degree)
std::vector<int> parameters;
void init_parameters() {
    for (int i = 0; i <= 20; i++) {
        parameters.push_back(i);
    }
}

void cleanup_resources(std::map<std::string, std::vector<double>>& time_sets) {
    for (auto& pair : time_sets) {
        std::vector<double>().swap(pair.second);
    }
    std::map<std::string, std::vector<double>>().swap(time_sets);
}

void approx_encoding() {
    std::string input_path = "../../datasets/revision/minibranch/processing_file/";
    std::string output_path = "../result_revision/minibranch/processing_file/approx/";

    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Store time results
    std::map<std::string, std::vector<double>> time_sets;
    std::vector<double> time_list;

    // Calculate total tasks for progress display
    size_t total_tasks = parameters.size();
    size_t current_task = 0;

    std::cout << "\n=== Processing Disturbing Degree Files ===" << std::endl;

    for (int p : parameters) {
        current_task++;
        std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                  << "Parameter: " << p << std::endl;

        std::string input_file_name = input_path + "Linux_" + std::to_string(p) + ".log";
        std::string output_file_name = output_path + "Linux_" + std::to_string(p);
        
        if (!std::ifstream(input_file_name)) {
            std::cerr << "Input file does not exist: " << input_file_name << std::endl;
            continue;
        }

        // Check if output file path is writable
        std::ofstream test_output(output_file_name);
        if (!test_output) {
            std::cerr << "Cannot write to output file: " << output_file_name << std::endl;
            continue;
        }
        test_output.close();

        // Wrap main_encoding_compress call in try-catch block
        try {
            double time = main_encoding_compress(input_file_name, output_file_name, 
                                               DefaultParams::WINDOW_SIZE,
                                               1,
                                               DefaultParams::BLOCK_SIZE,
                                               DefaultParams::COMPRESSOR,
                                               DefaultParams::DISTANCE,
                                               DefaultParams::USE_APPROX);
            time_list.push_back(time);
            std::cout << "Time cost: " << time << " seconds" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error processing parameter " << p 
                      << ": " << e.what() << std::endl;
            continue;
        }
    }

    // Store results in time_sets with a single key for CSV compatibility
    time_sets["Compression_Time"] = time_list;

    // Before writing to CSV, validate the data
    if (time_list.size() != parameters.size()) {
        std::cerr << "Warning: Incomplete data (expected " << parameters.size() 
                  << " entries, got " << time_list.size() << ")" << std::endl;
    }

    // Write results to CSV file
    std::string csv_path = output_path + "time_cost.csv";
    std::vector<std::string> first_column;
    first_column.push_back("Parameter");  // Header
    for (int p : parameters) {
        first_column.push_back(std::to_string(p));
    }

    std::cout << "Preparing to write CSV..." << std::endl;
    std::cout << "Number of parameters: " << parameters.size() << std::endl;
    std::cout << "Size of time_list: " << time_list.size() << std::endl;

    bool write_success = write_csv(csv_path, time_sets, {"Compression_Time"}, first_column);

    std::cout << "CSV write completed with status: " << (write_success ? "success" : "failure") << std::endl;

    // Finally clean up
    cleanup_resources(time_sets);
}

void exact_encoding() {
    std::string input_path = "../../datasets/revision/minibranch/processing_file/";
    std::string output_path = "../result_revision/minibranch/processing_file/exact/";

    // Ensure output directory exists
    if (!ensure_directory_exists(output_path)) {
        std::cerr << "Failed to create output directory: " << output_path << std::endl;
        return;
    }

    // Store time results
    std::map<std::string, std::vector<double>> time_sets;
    std::vector<double> time_list;

    // Calculate total tasks for progress display
    size_t total_tasks = parameters.size();
    size_t current_task = 0;

    std::cout << "\n=== Processing Disturbing Degree Files (Exact) ===" << std::endl;

    for (int p : parameters) {
        current_task++;
        std::cout << "Progress: [" << current_task << "/" << total_tasks << "] "
                  << "Parameter: " << p << std::endl;

        std::string input_file_name = input_path + "Linux_" + std::to_string(p) + ".log";
        std::string output_file_name = output_path + "Linux_" + std::to_string(p);
        
        if (!std::ifstream(input_file_name)) {
            std::cerr << "Input file does not exist: " << input_file_name << std::endl;
            continue;
        }

        // Check if output file path is writable
        std::ofstream test_output(output_file_name);
        if (!test_output) {
            std::cerr << "Cannot write to output file: " << output_file_name << std::endl;
            continue;
        }
        test_output.close();

        // Wrap main_encoding_compress call in try-catch block
        try {
            double time = main_encoding_compress(input_file_name, output_file_name, 
                                               DefaultParams::WINDOW_SIZE,
                                               1,
                                               DefaultParams::BLOCK_SIZE,
                                               DefaultParams::COMPRESSOR,
                                               DefaultParams::DISTANCE,
                                               false);  // Set use_approx to false
            time_list.push_back(time);
            std::cout << "Time cost: " << time << " seconds" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error processing parameter " << p 
                      << ": " << e.what() << std::endl;
            continue;
        }
    }

    // Store results in time_sets with a single key for CSV compatibility
    time_sets["Compression_Time"] = time_list;

    // Before writing to CSV, validate the data
    if (time_list.size() != parameters.size()) {
        std::cerr << "Warning: Incomplete data (expected " << parameters.size() 
                  << " entries, got " << time_list.size() << ")" << std::endl;
    }

    // Write results to CSV file
    std::string csv_path = output_path + "time_cost.csv";
    std::vector<std::string> first_column;
    first_column.push_back("Parameter");  // Header
    for (int p : parameters) {
        first_column.push_back(std::to_string(p));
    }

    bool write_success = write_csv(csv_path, time_sets, {"Compression_Time"}, first_column);

    if (write_success) {
        std::cout << "\nAll tasks completed. Results written to: " << csv_path << std::endl;
    } else {
        std::cerr << "\nFailed to write results to: " << csv_path << std::endl;
    }
    
    // Clean up resources
    cleanup_resources(time_sets);
}

int main() {
    try {
        // Initialize parameters
        init_parameters();
        
        std::cout << "\n=== Starting Approximate Encoding ===\n" << std::endl;
        approx_encoding();
        std::cout << "\n=== Approximate Encoding Completed ===\n" << std::endl;
        
        // Add a small delay to ensure resources are properly released
        std::cout.flush();
        
        std::cout << "\n=== Starting Exact Encoding ===\n" << std::endl;
        exact_encoding();
        
        std::cout << "\n=== All Tasks Completed ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}
