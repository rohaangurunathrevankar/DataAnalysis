#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <thread>
#include "data_analyzer.hpp"

using namespace car_sales;

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <csv_file> [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --chunk-size <n>   Set chunk size for processing (default: 10000)\n";
    std::cout << "  --threads <n>      Number of threads for concurrent processing (default: auto)\n";
    std::cout << "  --sequential       Disable concurrent processing\n";
    std::cout << "  --help             Show this help message\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << program_name << " data.csv\n";
    std::cout << "  " << program_name << " data.csv --threads 8\n";
    std::cout << "  " << program_name << " data.csv --chunk-size 5000 --sequential\n";
}

void printResults(const AnalysisResult& result) {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    CAR SALES ANALYSIS REPORT                     ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════════╣\n";
    
    // Task 1: Audi China 2025 Sales
    std::cout << "║                                                                  ║\n";
    std::cout << "║  1. AUDI CARS SOLD IN CHINA (2025)                               ║\n";
    std::cout << "║     ─────────────────────────────────                            ║\n";
    std::cout << "║     Total Units Sold: " << std::setw(10) << result.audi_china_year_sales 
              << "                              ║\n";
    
    // Task 2: BMW Total Revenue 2025
    std::cout << "║                                                                  ║\n";
    std::cout << "║  2. BMW TOTAL REVENUE (2025)                                     ║\n";
    std::cout << "║     ────────────────────────                                     ║\n";
    std::cout << "║     Total Revenue: $" << std::fixed << std::setprecision(2) 
              << std::setw(15) << result.bmw_year_total_revenue 
              << "                         ║\n";
    
    // Task 3: BMW European Revenue Distribution
    std::cout << "║                                                                  ║\n";
    std::cout << "║  3. BMW REVENUE DISTRIBUTION IN EUROPE (2025)                    ║\n";
    std::cout << "║     ─────────────────────────────────────────                    ║\n";
    
    if (result._bmw_europe_revenuedistribution.empty()) {
        std::cout << "║     No European sales data found                                 ║\n";
    } else {
        std::cout << "║     Country                           Revenue                    ║\n";
        std::cout << "║     ───────────────────────────────────────────                  ║\n";
        
        for (const auto& [country, revenue] : result._bmw_europe_revenuedistribution) {
            std::cout << "║     " << std::left << std::setw(25) << country 
                      << " $" << std::right << std::setw(15) << std::fixed << std::setprecision(2) 
                      << revenue << "         ║\n";
        }
    }
    
    // Processing Statistics
    std::cout << "║                                                                  ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  PROCESSING STATISTICS                                           ║\n";
    std::cout << "║  ───────────────────────                                         ║\n";
    std::cout << "║  Records Processed: " << std::setw(12) << result.total_records_processed 
              << "                              ║\n";
    std::cout << "║  Records Failed:    " << std::setw(12) << result.total_records_failed 
              << "                              ║\n";
    std::cout << "║  Analysis Status:   " << std::setw(12) 
              << (result.analysis_complete ? "Complete" : "Incomplete") 
              << "                              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";
    
    // Print errors if any (limited to first 10)
    if (!result.errors.empty()) {
        std::cout << "\nWarnings/Errors (first " << std::min(result.errors.size(), size_t(10)) << "):\n";
        for (size_t i = 0; i < std::min(result.errors.size(), size_t(10)); ++i) {
            std::cout << "  - " << result.errors[i] << "\n";
        }
        if (result.errors.size() > 10) {
            std::cout << "  ... and " << (result.errors.size() - 10) << " more errors\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string filename;
    size_t chunk_size = CsvParser::DEFAULT_CHUNK_SIZE;
    size_t num_threads = 0;  // 0 = auto-detect
    bool use_concurrent = true;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (std::strcmp(argv[i], "--chunk-size") == 0) {
            if (i + 1 < argc) {
                try {
                    chunk_size = std::stoul(argv[++i]);
                    if (chunk_size == 0) {
                        std::cerr << "Error: Chunk size must be greater than 0\n";
                        return 1;
                    }
                } catch (const std::exception&) {
                    std::cerr << "Error: Invalid chunk size value\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: --chunk-size requires a value\n";
                return 1;
            }
        } else if (std::strcmp(argv[i], "--threads") == 0) {
            if (i + 1 < argc) {
                try {
                    num_threads = std::stoul(argv[++i]);
                } catch (const std::exception&) {
                    std::cerr << "Error: Invalid thread count value\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: --threads requires a value\n";
                return 1;
            }
        } else if (std::strcmp(argv[i], "--sequential") == 0) {
            use_concurrent = false;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        } else {
            std::cerr << "Unknown option: " << argv[i] << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    if (filename.empty()) {
        std::cerr << "Error: No input file specified\n";
        printUsage(argv[0]);
        return 1;
    }
    
    // Auto-detect threads if not specified
    size_t detected_threads = num_threads;
    if (use_concurrent && num_threads == 0) {
        detected_threads = std::thread::hardware_concurrency();
        if (detected_threads == 0) detected_threads = 4;
    }
    
    std::cout << "Car Sales Analyzer \n";
    std::cout << "=============================================\n";
    std::cout << "Input file: " << filename << "\n";
    std::cout << "Chunk size: " << chunk_size << " records\n";
    std::cout << "Processing mode: " << (use_concurrent ? "Concurrent" : "Sequential") << "\n";
    if (use_concurrent) {
        std::cout << "Threads: " << detected_threads << "\n";
    }
    std::cout << "Processing...\n";
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        CarSalesAnalyzer analyzer(chunk_size);
        AnalysisResult result = analyzer.analyzeFile(filename, use_concurrent, num_threads);
        
        // End timing
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        printResults(result);
        
        std::cout << "\nProcessing time: " << duration.count() << " ms\n";
        
        if (result.total_records_processed > 0 && duration.count() > 0) {
            double records_per_second = static_cast<double>(result.total_records_processed) / 
                                        (static_cast<double>(duration.count()) / 1000.0);
            std::cout << "Processing speed: " << std::fixed << std::setprecision(0) 
                      << records_per_second << " records/second\n";
        }
        
        return result.analysis_complete ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
