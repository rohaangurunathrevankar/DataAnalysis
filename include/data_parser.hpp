#ifndef data__parserH
#define data__parserH

#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <stdexcept>
#include <optional>
#include <memory>
#include <thread>
#include <future>
#include <mutex>
#include <atomic>
#include <unordered_map>

namespace car_sales {

/**
 * @brief Represents a single car sale record
 * Adapted for data.csv format with tab-separated values
 */
struct CarSaleRecord {
  std::string brand;   // manufacturer column
  std::string country; // country column
  int year;            // extracted from sale_date (DD-MM-YYYY)
  int quantity;        // 1 per row (each row is one sale)
  double revenue;      // sale_price_usd column

  CarSaleRecord() : year(0), quantity(1), revenue(0.0) {}
  CarSaleRecord(const std::string &_brand, const std::string &_country,
                int _year, int _quantity, double _revenue)
      : brand(_brand), country(_country), year(_year), quantity(_quantity),
        revenue(_revenue) {}
};

/**
 * @brief Exception class for CSV parsing errors
 */
class CsvParseException : public std::runtime_error {
public:
  explicit CsvParseException(const std::string &message, size_t line_number = 0)
      : std::runtime_error(message), line_number_(line_number) {}

  size_t getLineNumber() const { return line_number_; }

private:
  size_t line_number_;
};

/**
 * @brief Result of a chunk processing operation
 */
struct ChunkResult {
  size_t records_processed;
  size_t records_failed;
  std::vector<std::string> errors;
  bool success;

  // Partial aggregation results for concurrent processing
  int audi_china_year_sales;
  double bmw_2025_revenue;
  std::unordered_map<std::string, double> bmw_europe_revenue;

  ChunkResult()
      : records_processed(0), records_failed(0), success(true),
        audi_china_year_sales(0), bmw_2025_revenue(0.0) {}
};

/**
 * @brief Callback type for processing chunks of records
 */
using ChunkProcessor =
    std::function<bool(const std::vector<CarSaleRecord> &, ChunkResult &)>;

/**
 * @brief CSV Parser with chunked reading support for large files
 *
 * Supports processing files with millions of records by reading in configurable
 * chunks. Default chunk size is 10,000 records as per requirements.
 */
class CsvParser {
public:
  static constexpr size_t DEFAULT_CHUNK_SIZE = 10000;

  explicit CsvParser(size_t chunk_size = DEFAULT_CHUNK_SIZE,
                     char delimiter = '\t');
  ~CsvParser() = default;

  // Prevent copying
  CsvParser(const CsvParser &) = delete;
  CsvParser &operator=(const CsvParser &) = delete;

  // Allow moving
  CsvParser(CsvParser &&) = default;
  CsvParser &operator=(CsvParser &&) = default;

  /**
   * @brief Parse a CSV file in chunks, calling the processor for each chunk
   * @param filename Path to the CSV file
   * @param processor Callback function to process each chunk
   * @return Overall result of the parsing operation
   */
  ChunkResult parseFile(const std::string &filename, ChunkProcessor processor);

  /**
   * @brief Parse a CSV file with concurrent chunk processing
   * @param filename Path to the CSV file
   * @param num_threads Number of worker threads (0 = auto-detect based on CPU
   * cores)
   * @return Aggregated ChunkResult with all analysis results
   */
  ChunkResult parseFileConcurrent(const std::string &filename,
                                  size_t num_threads = 0);

  /**
   * @brief Parse a single line into a CarSaleRecord
   * @param line The CSV line to parse
   * @return Parsed record or nullopt if parsing fails
   */
  std::optional<CarSaleRecord> parseLine(const std::string &line);

  /**
   * @brief Parse CSV content from a string (for testing)
   * @param content CSV content as a string
   * @param processor Callback function to process each chunk
   * @return Overall result of the parsing operation
   */
  ChunkResult parseString(const std::string &content, ChunkProcessor processor);

  /**
   * @brief Get the configured chunk size
   */
  size_t getChunkSize() const { return chunk_size_; }

  /**
   * @brief Set the chunk size for processing
   */
  void setChunkSize(size_t size) { chunk_size_ = size; }

  /**
   * @brief Get the delimiter character
   */
  char getDelimiter() const { return delimiter_; }

  /**
   * @brief Set the delimiter character
   */
  void setDelimiter(char d) { delimiter_ = d; }

  /**
   * @brief Get total records processed in last operation
   */
  size_t getTotalRecordsProcessed() const { return _total_records_processed; }

private:
  size_t chunk_size_;
  size_t _total_records_processed;
  char delimiter_;

  std::vector<std::string> splitLine(const std::string &line, char delimiter);
  std::string trim(const std::string &str);
  bool isNumeric(const std::string &str);

  /**
   * @brief Extract year from date string in DD-MM-YYYY format
   */
  int extractYearFromDate(const std::string &date_str);

  /**
   * @brief Process a single chunk and update partial results
   */
  static void processChunkAnalysis(const std::vector<CarSaleRecord> &chunk,
                                   ChunkResult &result);

  /**
   * @brief Merge partial results from multiple threads
   */
  static void mergeResults(ChunkResult &target, const ChunkResult &source);
};

} // namespace car_sales

#endif // data__parserH