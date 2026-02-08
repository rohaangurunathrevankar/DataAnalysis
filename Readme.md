# DataProcessor: Car Sales Analysis Tool

**DataProcessor** is a high-performance C++ utility designed to parse and analyze large-scale CSV datasets. It utilizes chunk-based processing and concurrent execution to handle massive files efficiently, optimizing memory usage while performing complex aggregations and financial calculations.

## 📂 Project Structure

The project is organized as follows:

```text
DataProcessor
├── CMakeLists.txt           # CMake build configuration
├── Readme.md                # Project documentation and execution steps
├── include/                 # Header files
│   ├── data_parser.hpp      # CSV file parsing interface
│   └── data_analyzer.hpp    # Data analysis interface
├── src/                     # Source files
│   ├── data_parser.cpp      # CSV parsing implementation
│   └── data_analyzer.cpp    # Analysis logic implementation
├── test/                    # Unit tests
│   ├── test_data_parser.cpp # Tests for CSV parsing logic
│   └── test_data_analyzer.cpp # Tests for analysis calculations
└── data/
    └── sample.csv           # Sample dataset for testing
============================================================================================

Execution steps:

make build && cd build
cmake ..
cmake --build .
make

./data_analyzer data.csv # basic usage with default 10k chunksize
============================================================================================
Success Scenario :
Input file: data/sample_data.csv
Chunk size: 10000 records
Processing...

╔══════════════════════════════════════════════════════════════════╗
║                    CAR SALES ANALYSIS REPORT                     ║
╠══════════════════════════════════════════════════════════════════╣
║                                                                  ║
║  1. AUDI CARS SOLD IN CHINA (2025)                               ║
║     ─────────────────────────────────                            ║
║     Total Units Sold:       4600                                 ║
║                                                                  ║
║  2. BMW TOTAL REVENUE (2025)                                     ║
║     ────────────────────────                                     ║
║     Total Revenue: $    950000000.00                             ║
║                                                                  ║
║  3. BMW REVENUE DISTRIBUTION IN EUROPE (2025)                    ║
║     ─────────────────────────────────────────                    ║
║     Country                           Revenue                    ║
║     ───────────────────────────────────────────                  ║
║     Germany                  $    100000000.00                   ║
║     United Kingdom           $     90000000.00                   ║
║     France                   $     75000000.00                   ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝

Processing time: 15 ms
Processing speed: 3333 records/second
============================================================================================
Failure Scenario :
Input file: data.csv
Chunk size: 10000 records
Processing mode: Concurrent
Threads: 10
Processing...

╔══════════════════════════════════════════════════════════════════╗
║                    CAR SALES ANALYSIS REPORT                     ║
╠══════════════════════════════════════════════════════════════════╣
║                                                                  ║
║  1. AUDI CARS SOLD IN CHINA (2025)                               ║
║     ─────────────────────────────────                            ║
║     Total Units Sold:          0                              ║
║                                                                  ║
║  2. BMW TOTAL REVENUE (2025)                                     ║
║     ────────────────────────                                     ║
║     Total Revenue: $           0.00                         ║
║                                                                  ║
║  3. BMW REVENUE DISTRIBUTION IN EUROPE (2025)                    ║
║     ─────────────────────────────────────────                    ║
║     No European sales data found                                 ║
║                                                                  ║
╠══════════════════════════════════════════════════════════════════╣
║  PROCESSING STATISTICS                                           ║
║  ───────────────────────                                         ║
║  Records Processed:            0                                 ║
║  Records Failed:               0                                 ║
║  Analysis Status:     Incomplete                                 ║
╚══════════════════════════════════════════════════════════════════╝


./data_analyzer data.csv --chunk-size 5000

test execution
./car_sales_tests


