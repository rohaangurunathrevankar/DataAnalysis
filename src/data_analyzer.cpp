#include <cctype>

#include "data_analyzer.hpp"

namespace car_sales {

// European countries for BMW revenue distribution analysis
const std::set<std::string> CarSalesAnalyzer::EUROPEAN_COUNTRIES = {
    "Germany",
    "France",
    "United Kingdom",
    "UK",
    "Italy",
    "Spain",
    "Netherlands",
    "Belgium",
    "Austria",
    "Switzerland",
    "Sweden",
    "Norway",
    "Denmark",
    "Finland",
    "Poland",
    "Czech Republic",
    "Portugal",
    "Greece",
    "Ireland",
    "Hungary",
    "Romania",
    "Bulgaria",
    "Croatia",
    "Slovakia",
    "Slovenia",
    "Lithuania",
    "Latvia",
    "Estonia",
    "Luxembourg",
    "Malta",
    "Cyprus",
    "Iceland",
    "Serbia",
    "Montenegro",
    "North Macedonia",
    "Albania",
    "Bosnia and Herzegovina",
    "Moldova",
    "Ukraine",
    "Belarus",
    "Russia"};

CarSalesAnalyzer::CarSalesAnalyzer(size_t chunk_size)
    : _parser(std::make_unique<CsvParser>(chunk_size)),
      _audi_china_year_sales(0), _bmw_2025_revenue(0.0),
      _total_records_processed(0), _total_records_failed(0) {}

bool CarSalesAnalyzer::isEuropeanCountry(const std::string &country) {
  return EUROPEAN_COUNTRIES.find(country) != EUROPEAN_COUNTRIES.end();
}

void CarSalesAnalyzer::reset() {
  _audi_china_year_sales = 0;
  _bmw_2025_revenue = 0.0;
  _bmw_europe_revenue.clear();
  _total_records_processed = 0;
  _total_records_failed = 0;
  _errors.clear();
}

void CarSalesAnalyzer::processRecord(const CarSaleRecord &record) {
  // Task 1: Count Audi cars sold in China in 2025
  if (record.brand == "Audi" && record.country == "China" &&
      record.year == 2025) {
    _audi_china_year_sales += record.quantity;
  }

  // Task 2 & 3: BMW analysis for 2025
  if (record.brand == "BMW" && record.year == 2025) {
    // Total BMW revenue in 2025
    _bmw_2025_revenue += record.revenue;

    // BMW revenue in European countries
    if (isEuropeanCountry(record.country)) {
      _bmw_europe_revenue[record.country] += record.revenue;
    }
  }
}

void CarSalesAnalyzer::processChunk(const std::vector<CarSaleRecord> &records) {
  for (const auto &record : records) {
    processRecord(record);
  }
  _total_records_processed += records.size();
}

std::vector<std::pair<std::string, double>>
CarSalesAnalyzer::getBmwEuropeRevenueDistribution() const {
  std::vector<std::pair<std::string, double>> distribution(
      _bmw_europe_revenue.begin(), _bmw_europe_revenue.end());

  // Sort by revenue in descending order
  std::sort(distribution.begin(), distribution.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  return distribution;
}

AnalysisResult CarSalesAnalyzer::getResults() const {
  AnalysisResult result;
  result.audi_china_year_sales = _audi_china_year_sales;
  result.bmw_year_total_revenue = _bmw_2025_revenue;
  result._bmw_europe_revenuedistribution = getBmwEuropeRevenueDistribution();
  result.total_records_processed = _total_records_processed;
  result.total_records_failed = _total_records_failed;
  result.errors = _errors;
  result.analysis_complete = true;
  return result;
}

AnalysisResult CarSalesAnalyzer::analyzeFile(const std::string &filename,
                                             bool use_concurrent,
                                             size_t num_threads) {
  reset();

  if (use_concurrent) {
    // Use concurrent processing
    ChunkResult parse_result =
        _parser->parseFileConcurrent(filename, num_threads);

    // Transfer results from concurrent processing
    _audi_china_year_sales = parse_result.audi_china_year_sales;
    _bmw_2025_revenue = parse_result.bmw_2025_revenue;
    _bmw_europe_revenue = parse_result.bmw_europe_revenue;
    _total_records_processed = parse_result.records_processed;
    _total_records_failed = parse_result.records_failed;
    _errors = parse_result.errors;

    if (!parse_result.success) {
      AnalysisResult result = getResults();
      result.analysis_complete = false;
      return result;
    }

    return getResults();
  }

  // Sequential processing
  auto processor = [this](const std::vector<CarSaleRecord> &chunk,
                          ChunkResult &result) -> bool {
    try {
      processChunk(chunk);
      result.success = true;
      result.records_processed = chunk.size();
      return true;
    } catch (const std::exception &e) {
      result.success = false;
      result.errors.push_back(std::string("Processing error: ") + e.what());
      return false;
    }
  };

  ChunkResult parse_result = _parser->parseFile(filename, processor);

  _total_records_failed = parse_result.records_failed;
  _errors = parse_result.errors;

  if (!parse_result.success) {
    AnalysisResult result = getResults();
    result.analysis_complete = false;
    return result;
  }

  return getResults();
}

AnalysisResult CarSalesAnalyzer::analyzeString(const std::string &content) {
  reset();

  auto processor = [this](const std::vector<CarSaleRecord> &chunk,
                          ChunkResult &result) -> bool {
    try {
      processChunk(chunk);
      result.success = true;
      result.records_processed = chunk.size();
      return true;
    } catch (const std::exception &e) {
      result.success = false;
      result.errors.push_back(std::string("Processing error: ") + e.what());
      return false;
    }
  };

  ChunkResult parse_result = _parser->parseString(content, processor);

  _total_records_failed = parse_result.records_failed;
  _errors = parse_result.errors;

  if (!parse_result.success) {
    AnalysisResult result = getResults();
    result.analysis_complete = false;
    return result;
  }

  return getResults();
}

} // namespace car_sales