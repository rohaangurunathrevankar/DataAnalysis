cmake_minimum_required(VERSION 3.14)
project(CarSalesAnalyzer VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find threading library
find_package(Threads REQUIRED)

# Enable testing
enable_testing()

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Library for CSV parsing and analysis
add_library(car_sales_lib
    src/data_parser.cpp
    src/data_analyzer.cpp
)

target_include_directories(car_sales_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(car_sales_lib PUBLIC Threads::Threads)

# Main executable
add_executable(data_analyzer src/main.cpp)
target_link_libraries(data_analyzer PRIVATE car_sales_lib)

# Test executable
add_executable(car_sales_tests
    test/test_data_parser.cpp
    test/test_data_analyzer.cpp
)

target_link_libraries(car_sales_tests PRIVATE 
    car_sales_lib 
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(car_sales_tests)

# Install targets
install(TARGETS data_analyzer DESTINATION bin)
