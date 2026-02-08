#ifndef data_analyzer_HPP
#define data_analyzer_HPP

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "data_parser.hpp"

namespace car_sales {

/**
 * @brief Analysis result containing all computed metrics
 */
struct AnalysisResult {
  // Audi sales in China 2025
  int audi_china_year_sales;

  // BMW total revenue 2025
  double bmw_year_total_revenue;

  // BMW revenue by European country (sorted highest to lowest)
  std::vector<std::pair<std::string, double>> _bmw_europe_revenuedistribution;

  // Processing statistics
  size_t total_records_processed;
  size_t total_records_failed;
  bool analysis_complete;
  std::vector<std::string> errors;

  AnalysisResult()
      : audi_china_year_sales(0), bmw_year_total_revenue(0.0),
        total_records_processed(0), total_records_failed(0),
        analysis_complete(false) {}
};

/**
 * @brief Analyzes car sales data from CSV files
 *
 * - Audi sales in China for 2025
 * - BMW total revenue for 2025
 * - BMW revenue distribution across European countries
 */
class CarSalesAnalyzer {
public:
  // European countries for filtering
  static const std::set<std::string> EUROPEAN_COUNTRIES;

  explicit CarSalesAnalyzer(size_t chunk_size = CsvParser::DEFAULT_CHUNK_SIZE);
  ~CarSalesAnalyzer() = default;

  /**
   * @brief Analyze a CSV file and compute all metrics
   * @param filename Path to the CSV file
   * @param use_concurrent Use multi-threaded processing (default: true)
   * @param num_threads Number of threads (0 = auto-detect)
   * @return Analysis results
   */
  AnalysisResult analyzeFile(const std::string &filename,
                             bool use_concurrent = true,
                             size_t num_threads = 0);

  /**
   * @brief Analyze CSV content from a string (for testing)
   * @param content CSV content as a string
   * @return Analysis results
   */
  AnalysisResult analyzeString(const std::string &content);

  /**
   * @brief Process a chunk of records (can be called directly for streaming)
   * @param records Vector of car sale records
   */
  void processChunk(const std::vector<CarSaleRecord> &records);

  /**
   * @brief Get current accumulated results
   * @return Current analysis results
   */
  AnalysisResult getResults() const;

  /**
   * @brief Reset all accumulated data
   */
  void reset();

  /**
   * @brief Get Audi sales count in China for year 2025
   */
  int getAudiChinaSales2025() const { return _audi_china_year_sales; }

  /**
   * @brief Get BMW total revenue for year 2025
   */
  double getBmw2025Revenue() const { return _bmw_2025_revenue; }

  /**
   * @brief Get BMW revenue distribution across European countries
   * @return Vector of country-revenue pairs sorted by revenue (descending)
   */
  std::vector<std::pair<std::string, double>>
  getBmwEuropeRevenueDistribution() const;

  /**
   * @brief Check if a country is in Europe
   */
  static bool isEuropeanCountry(const std::string &country);

private:
  std::unique_ptr<CsvParser> _parser;

  // Accumulated metrics
  int _audi_china_year_sales;
  double _bmw_2025_revenue;
  std::unordered_map<std::string, double> _bmw_europe_revenue;

  // Statistics
  size_t _total_records_processed;
  size_t _total_records_failed;
  std::vector<std::string> _errors;

  void processRecord(const CarSaleRecord &record);
};

} // namespace car_sales

#endif // data_analyzer_HPP