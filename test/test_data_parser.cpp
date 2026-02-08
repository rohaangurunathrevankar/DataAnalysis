#include "data_parser.hpp"
#include <gtest/gtest.h>

using namespace car_sales;

class CsvParserTest : public ::testing::Test {
protected:
  void SetUp() override {
    parser = std::make_unique<CsvParser>(
        100, '\t'); // Tab delimiter, small chunk for testing
  }

  std::unique_ptr<CsvParser> parser;

  std::string createLine(const std::string &sale_date,
                         const std::string &country,
                         const std::string &manufacturer, double sale_price) {
    std::string line =
        "SALE001\t" + sale_date + "\t" + country + "\tRegion\t0.0\t0.0\t";
    line += "D001\tDealer 1\t" + manufacturer +
            "\tModel\t2025\tSedan\tPetrol\tAutomatic\t";
    line += "AWD\tBlack\tVIN123\tNew\t0\t0\t" + std::to_string(sale_price) +
            "\tUSD\t";
    line += "TRUE\tLease\tIn-store\tB001\t35\tMale\t75000\tS001\tSales 1\t48\t";
    line += "Manufacturer\tFeatures\t120\t25\t32\t2.0\t201\t280\t4.5\t\tFALSE";
    return line;
  }
};

// ============================================================================
// Basic Line Parsing Tests
// ============================================================================

TEST_F(CsvParserTest, ParseValidLine) {
  std::string line = createLine("15-01-2025", "China", "Audi", 45000);
  auto result = parser->parseLine(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->brand, "Audi");
  EXPECT_EQ(result->country, "China");
  EXPECT_EQ(result->year, 2025);
  EXPECT_EQ(result->quantity, 1); // Each row = 1 sale
  EXPECT_DOUBLE_EQ(result->revenue, 45000);
}

TEST_F(CsvParserTest, ParseLineWithDifferentManufacturer) {
  std::string line = createLine("20-02-2025", "Germany", "BMW", 75000);
  auto result = parser->parseLine(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->brand, "BMW");
  EXPECT_EQ(result->country, "Germany");
  EXPECT_EQ(result->year, 2025);
}

TEST_F(CsvParserTest, ParseEmptyLine) {
  auto result = parser->parseLine("");
  EXPECT_FALSE(result.has_value());
}

TEST_F(CsvParserTest, ParseLineWithMissingFields) {
  auto result = parser->parseLine("SALE001\t15-01-2025\tChina");
  EXPECT_FALSE(result.has_value());
}

TEST_F(CsvParserTest, ExtractYearFromDate) {
  std::string line = createLine("15-03-2025", "France", "Mercedes", 50000);
  auto result = parser->parseLine(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->year, 2025);
}

TEST_F(CsvParserTest, ExtractYearFrom2024) {
  // Modify the date to 2024
  std::string line = createLine("20-06-2024", "USA", "Ford", 35000);
  auto result = parser->parseLine(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->year, 2024);
}

// ============================================================================
// String Parsing Tests
// ============================================================================

TEST_F(CsvParserTest, ParseStringWithHeader) {
  std::string csv =
      "sale_id\tsale_date\tcountry\tregion\tlatitude\tlongitude\t";
  csv += "dealership_id\tdealership_name\tmanufacturer\tmodel\tvehicle_year\t";
  csv +=
      "body_type\tfuel_type\ttransmission\tdrivetrain\tcolor\tvin\tcondition\t";
  csv += "previous_owners\todometer_km\tsale_price_usd\tcurrency\tfinancing\t";
  csv += "payment_type\tsales_channel\tbuyer_id\tbuyer_age\tbuyer_gender\t";
  csv +=
      "buyer_income_usd\tsalesperson_id\tsalesperson_name\twarranty_months\t";
  csv += "warranty_provider\tfeatures\tco2_g_km\tmpg_city\tmpg_highway\t";
  csv += "engine_displacement_l\thorsepower\ttorque_nm\tdealer_rating\t";
  csv += "condition_notes\tservice_history\n";
  csv += createLine("15-01-2025", "China", "Audi", 45000) + "\n";
  csv += createLine("20-02-2025", "Germany", "BMW", 75000) + "\n";

  std::vector<CarSaleRecord> all_records;
  auto result = parser->parseString(
      csv,
      [&all_records](const std::vector<CarSaleRecord> &chunk, ChunkResult &) {
        all_records.insert(all_records.end(), chunk.begin(), chunk.end());
        return true;
      });

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.records_processed, 2);
  EXPECT_EQ(all_records.size(), 2);
}

TEST_F(CsvParserTest, ParseEmptyString) {
  std::string csv = "";

  std::vector<CarSaleRecord> all_records;
  auto result = parser->parseString(
      csv,
      [&all_records](const std::vector<CarSaleRecord> &chunk, ChunkResult &) {
        all_records.insert(all_records.end(), chunk.begin(), chunk.end());
        return true;
      });

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.records_processed, 0);
}

// ============================================================================
// Chunking Tests
// ============================================================================

TEST_F(CsvParserTest, ChunkingWithSmallChunkSize) {
  CsvParser small_chunk_parser(3, '\t'); // Very small chunk size

  std::string header =
      "sale_id\tsale_date\tcountry\tregion\tlatitude\tlongitude\t";
  header +=
      "dealership_id\tdealership_name\tmanufacturer\tmodel\tvehicle_year\t";
  header +=
      "body_type\tfuel_type\ttransmission\tdrivetrain\tcolor\tvin\tcondition\t";
  header +=
      "previous_owners\todometer_km\tsale_price_usd\tcurrency\tfinancing\t";
  header += "payment_type\tsales_channel\tbuyer_id\tbuyer_age\tbuyer_gender\t";
  header +=
      "buyer_income_usd\tsalesperson_id\tsalesperson_name\twarranty_months\t";
  header += "warranty_provider\tfeatures\tco2_g_km\tmpg_city\tmpg_highway\t";
  header += "engine_displacement_l\thorsepower\ttorque_nm\tdealer_rating\t";
  header += "condition_notes\tservice_history\n";

  std::string csv = header;
  for (int i = 0; i < 10; ++i) {
    csv += createLine("15-01-2025", "China", "Audi", 45000 + i) + "\n";
  }

  int chunk_count = 0;
  auto result = small_chunk_parser.parseString(
      csv,
      [&chunk_count](const std::vector<CarSaleRecord> &chunk, ChunkResult &) {
        chunk_count++;
        return true;
      });

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.records_processed, 10);
  EXPECT_EQ(chunk_count, 4); // 3 + 3 + 3 + 1 = 4 chunks
}

TEST_F(CsvParserTest, ChunkSizeConfiguration) {
  CsvParser parser1(1000, '\t');
  EXPECT_EQ(parser1.getChunkSize(), 1000);

  parser1.setChunkSize(5000);
  EXPECT_EQ(parser1.getChunkSize(), 5000);
}

TEST_F(CsvParserTest, DefaultChunkSize) {
  CsvParser default_parser;
  EXPECT_EQ(default_parser.getChunkSize(), CsvParser::DEFAULT_CHUNK_SIZE);
}

TEST_F(CsvParserTest, DelimiterConfiguration) {
  CsvParser tab_parser(100, '\t');
  EXPECT_EQ(tab_parser.getDelimiter(), '\t');

  CsvParser comma_parser(100, ',');
  EXPECT_EQ(comma_parser.getDelimiter(), ',');
}

// ============================================================================
// Failure Handling Tests
// ============================================================================

TEST_F(CsvParserTest, ProcessorFailure) {
  std::string header =
      "sale_id\tsale_date\tcountry\tregion\tlatitude\tlongitude\t";
  header +=
      "dealership_id\tdealership_name\tmanufacturer\tmodel\tvehicle_year\t";
  header +=
      "body_type\tfuel_type\ttransmission\tdrivetrain\tcolor\tvin\tcondition\t";
  header +=
      "previous_owners\todometer_km\tsale_price_usd\tcurrency\tfinancing\t";
  header += "payment_type\tsales_channel\tbuyer_id\tbuyer_age\tbuyer_gender\t";
  header +=
      "buyer_income_usd\tsalesperson_id\tsalesperson_name\twarranty_months\t";
  header += "warranty_provider\tfeatures\tco2_g_km\tmpg_city\tmpg_highway\t";
  header += "engine_displacement_l\thorsepower\ttorque_nm\tdealer_rating\t";
  header += "condition_notes\tservice_history\n";

  std::string csv =
      header + createLine("15-01-2025", "China", "Audi", 45000) + "\n";

  auto result = parser->parseString(
      csv, [](const std::vector<CarSaleRecord> &, ChunkResult &) {
        return false; // Simulate processing failure
      });

  EXPECT_FALSE(result.success);
}

TEST_F(CsvParserTest, FileNotFound) {
  auto result = parser->parseFile(
      "/nonexistent/path/to/file.csv",
      [](const std::vector<CarSaleRecord> &, ChunkResult &) { return true; });

  EXPECT_FALSE(result.success);
  EXPECT_FALSE(result.errors.empty());
}


