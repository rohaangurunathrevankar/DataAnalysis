This project does analysis of csv data.

DataProcessor
-CMakeLists.txt #cmake build config
-Readme.md #execution steps
-include
    -data_parser.hpp.  #for Csv file parsing header file
    -data_analyzer.hpp #nalsysis header file
-src
    -data_parser.cpp.  #for Csv file parsing source file
    -data_analyzer.cpp #nalsysis source file
-test
    -test_data_parser.cpp. #to test parsing csv logic
    -test_data_analyzer.cpp #to test analysis logic
-data/sample.csv
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


