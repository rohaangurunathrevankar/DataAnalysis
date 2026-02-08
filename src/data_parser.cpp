#include <cctype>
#include <set>

#include "data_parser.hpp"

namespace car_sales {

// European countries for BMW revenue distribution
static const std::set<std::string> EUROPEAN_COUNTRIES = {
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

static bool isEuropeanCountry(const std::string &country) {
  return EUROPEAN_COUNTRIES.find(country) != EUROPEAN_COUNTRIES.end();
}

CsvParser::CsvParser(size_t chunk_size, char delimiter)
    : chunk_size_(chunk_size), _total_records_processed(0),
      delimiter_(delimiter) {
  if (chunk_size_ == 0) {
    chunk_size_ = DEFAULT_CHUNK_SIZE;
  }
}

int CsvParser::extractYearFromDate(const std::string &date_str) {
  // Format: DD-MM-YYYY
  if (date_str.length() < 10) {
    return 0;
  }

  // Find the last dash and extract year after it
  size_t last_dash = date_str.rfind('-');
  if (last_dash == std::string::npos || last_dash + 1 >= date_str.length()) {
    return 0;
  }

  try {
    std::string year_str = date_str.substr(last_dash + 1, 4);
    return std::stoi(year_str);
  } catch (...) {
    return 0;
  }
}

std::string CsvParser::trim(const std::string &str) {
  size_t start = 0;
  size_t end = str.length();

  while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
    ++start;
  }

  while (end > start &&
         std::isspace(static_cast<unsigned char>(str[end - 1]))) {
    --end;
  }

  return str.substr(start, end - start);
}

std::vector<std::string> CsvParser::splitLine(const std::string &line,
                                              char delimiter) {
  std::vector<std::string> fields;
  std::string field;
  bool in_quotes = false;

  for (size_t i = 0; i < line.length(); ++i) {
    char c = line[i];

    if (c == '"') {
      in_quotes = !in_quotes;
    } else if (c == delimiter && !in_quotes) {
      fields.push_back(trim(field));
      field.clear();
    } else {
      field += c;
    }
  }

  // Don't forget the last field
  fields.push_back(trim(field));

  return fields;
}

bool CsvParser::isNumeric(const std::string &str) {
  if (str.empty())
    return false;

  size_t start = 0;
  if (str[0] == '-' || str[0] == '+') {
    start = 1;
  }

  bool has_dot = false;
  for (size_t i = start; i < str.length(); ++i) {
    if (str[i] == '.') {
      if (has_dot)
        return false;
      has_dot = true;
    } else if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
      return false;
    }
  }

  return start < str.length();
}

std::optional<CarSaleRecord> CsvParser::parseLine(const std::string &line) {
  if (line.empty()) {
    return std::nullopt;
  }

  auto fields = splitLine(line, delimiter_);

  // data.csv format (tab-separated, 42+ columns):
  // 0: sale_id, 1: sale_date (DD-MM-YYYY), 2: country, 3: region, ...
  // 8: manufacturer, 9: model, 10: vehicle_year, ...
  // 20: sale_price_usd, ...

  if (fields.size() < 21) {
    return std::nullopt;
  }

  try {
    CarSaleRecord record;
    record.brand = fields[8];                     // manufacturer
    record.country = fields[2];                   // country
    record.year = extractYearFromDate(fields[1]); // sale_date
    record.quantity = 1;                          // each row is one sale

    // Parse sale_price_usd
    if (!isNumeric(fields[20])) {
      return std::nullopt;
    }
    record.revenue = std::stod(fields[20]);

    // Basic validation
    if (record.brand.empty() || record.country.empty()) {
      return std::nullopt;
    }

    if (record.year < 1900 || record.year > 2100) {
      return std::nullopt;
    }

    return record;

  } catch (const std::exception &) {
    return std::nullopt;
  }
}

ChunkResult CsvParser::parseFile(const std::string &filename,
                                 ChunkProcessor processor) {
  ChunkResult overall_result;
  _total_records_processed = 0;

  std::ifstream file(filename);
  if (!file.is_open()) {
    overall_result.success = false;
    overall_result.errors.push_back("Failed to open file: " + filename);
    return overall_result;
  }

  std::string line;
  std::vector<CarSaleRecord> chunk;
  chunk.reserve(chunk_size_);

  size_t line_number = 0;
  bool is_header = true;

  while (std::getline(file, line)) {
    ++line_number;

    // Skip header line
    if (is_header) {
      is_header = false;
      continue;
    }

    // Skip empty lines
    if (trim(line).empty()) {
      continue;
    }

    auto record = parseLine(line);
    if (record) {
      chunk.push_back(*record);
    } else {
      overall_result.records_failed++;
      if (overall_result.errors.size() < 100) { // Limit error messages
        overall_result.errors.push_back(
            "Failed to parse line " + std::to_string(line_number) + ": " +
            (line.length() > 50 ? line.substr(0, 50) + "..." : line));
      }
    }

    // Process chunk when full
    if (chunk.size() >= chunk_size_) {
      ChunkResult chunk_result;
      if (!processor(chunk, chunk_result)) {
        overall_result.success = false;
        overall_result.errors.push_back("Chunk processing failed at line " +
                                        std::to_string(line_number));
      }
      overall_result.records_processed += chunk.size();
      _total_records_processed += chunk.size();
      chunk.clear();
    }
  }

  // Process remaining records
  if (!chunk.empty()) {
    ChunkResult chunk_result;
    if (!processor(chunk, chunk_result)) {
      overall_result.success = false;
      overall_result.errors.push_back("Final chunk processing failed");
    }
    overall_result.records_processed += chunk.size();
    _total_records_processed += chunk.size();
  }

  file.close();
  return overall_result;
}

ChunkResult CsvParser::parseString(const std::string &content,
                                   ChunkProcessor processor) {
  ChunkResult overall_result;
  _total_records_processed = 0;

  std::istringstream stream(content);
  std::string line;
  std::vector<CarSaleRecord> chunk;
  chunk.reserve(chunk_size_);

  size_t line_number = 0;
  bool is_header = true;

  while (std::getline(stream, line)) {
    ++line_number;

    // Skip header line
    if (is_header) {
      is_header = false;
      continue;
    }

    // Skip empty lines
    if (trim(line).empty()) {
      continue;
    }

    auto record = parseLine(line);
    if (record) {
      chunk.push_back(*record);
    } else {
      overall_result.records_failed++;
      if (overall_result.errors.size() < 100) {
        overall_result.errors.push_back("Failed to parse line " +
                                        std::to_string(line_number));
      }
    }

    // Process chunk when full
    if (chunk.size() >= chunk_size_) {
      ChunkResult chunk_result;
      if (!processor(chunk, chunk_result)) {
        overall_result.success = false;
      }
      overall_result.records_processed += chunk.size();
      _total_records_processed += chunk.size();
      chunk.clear();
    }
  }

  // Process remaining records
  if (!chunk.empty()) {
    ChunkResult chunk_result;
    if (!processor(chunk, chunk_result)) {
      overall_result.success = false;
    }
    overall_result.records_processed += chunk.size();
    _total_records_processed += chunk.size();
  }

  return overall_result;
}

void CsvParser::processChunkAnalysis(const std::vector<CarSaleRecord> &chunk,
                                     ChunkResult &result) {
  for (const auto &record : chunk) {
    // Task 1: Count Audi cars sold in China in 2025
    if (record.brand == "Audi" && record.country == "China" &&
        record.year == 2025) {
      result.audi_china_year_sales += record.quantity;
    }

    // Task 2 & 3: BMW analysis for 2025
    if (record.brand == "BMW" && record.year == 2025) {
      result.bmw_2025_revenue += record.revenue;

      if (isEuropeanCountry(record.country)) {
        result.bmw_europe_revenue[record.country] += record.revenue;
      }
    }
  }
  result.records_processed = chunk.size();
}

void CsvParser::mergeResults(ChunkResult &target, const ChunkResult &source) {
  target.audi_china_year_sales += source.audi_china_year_sales;
  target.bmw_2025_revenue += source.bmw_2025_revenue;
  target.records_processed += source.records_processed;
  target.records_failed += source.records_failed;

  for (const auto &[country, revenue] : source.bmw_europe_revenue) {
    target.bmw_europe_revenue[country] += revenue;
  }

  if (!source.success) {
    target.success = false;
  }
}

ChunkResult CsvParser::parseFileConcurrent(const std::string &filename,
                                           size_t num_threads) {
  ChunkResult overall_result;
  _total_records_processed = 0;

  // Determine number of threads
  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0)
      num_threads = 4; // Default fallback
  }

  std::ifstream file(filename);
  if (!file.is_open()) {
    overall_result.success = false;
    overall_result.errors.push_back("Failed to open file: " + filename);
    return overall_result;
  }

  // Read all lines first (for large files, this could be optimized further)
  std::vector<std::string> all_lines;
  std::string line;
  bool is_header = true;

  while (std::getline(file, line)) {
    if (is_header) {
      is_header = false;
      continue;
    }
    if (!trim(line).empty()) {
      all_lines.push_back(std::move(line));
    }
  }
  file.close();

  if (all_lines.empty()) {
    overall_result.success = true;
    return overall_result;
  }

  // Parse all lines into records
  std::vector<CarSaleRecord> all_records;
  all_records.reserve(all_lines.size());

  for (size_t i = 0; i < all_lines.size(); ++i) {
    auto record = parseLine(all_lines[i]);
    if (record) {
      all_records.push_back(*record);
    } else {
      overall_result.records_failed++;
    }
  }

  // Clear lines to free memory
  all_lines.clear();
  all_lines.shrink_to_fit();

  if (all_records.empty()) {
    overall_result.success = true;
    return overall_result;
  }

  // Divide records into chunks for parallel processing
  size_t total_records = all_records.size();
  size_t records_per_thread = (total_records + num_threads - 1) / num_threads;

  std::vector<std::future<ChunkResult>> futures;
  futures.reserve(num_threads);

  for (size_t t = 0; t < num_threads; ++t) {
    size_t start_idx = t * records_per_thread;
    if (start_idx >= total_records)
      break;

    size_t end_idx = std::min(start_idx + records_per_thread, total_records);

    // Create a chunk for this thread
    std::vector<CarSaleRecord> chunk(all_records.begin() + start_idx,
                                     all_records.begin() + end_idx);

    // Launch async task
    futures.push_back(
        std::async(std::launch::async, [chunk = std::move(chunk)]() mutable {
          ChunkResult result;
          processChunkAnalysis(chunk, result);
          result.success = true;
          return result;
        }));
  }

  // Collect results from all threads
  for (auto &future : futures) {
    try {
      ChunkResult partial = future.get();
      mergeResults(overall_result, partial);
    } catch (const std::exception &e) {
      overall_result.success = false;
      overall_result.errors.push_back(std::string("Thread error: ") + e.what());
    }
  }

  _total_records_processed = overall_result.records_processed;
  overall_result.success = true;

  return overall_result;
}

} // namespace car_sales