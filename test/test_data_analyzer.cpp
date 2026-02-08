#include "data_analyzer.hpp"
#include <gtest/gtest.h>

using namespace car_sales;

class CarSalesAnalyzerTest : public ::testing::Test {
protected:
  void SetUp() override {
    analyzer =
        std::make_unique<CarSalesAnalyzer>(100); // Small chunk for testing
  }

  std::unique_ptr<CarSalesAnalyzer> analyzer;

  // Helper to create a header line
  std::string createHeader() {
    return "sale_id\tsale_date\tcountry\tregion\tlatitude\tlongitude\t"
           "dealership_id\tdealership_name\tmanufacturer\tmodel\tvehicle_year\t"
           "body_type\tfuel_"
           "type\ttransmission\tdrivetrain\tcolor\tvin\tcondition\t"
           "previous_owners\todometer_km\tsale_price_usd\tcurrency\tfinancing\t"
           "payment_type\tsales_channel\tbuyer_id\tbuyer_age\tbuyer_gender\t"
           "buyer_income_usd\tsalesperson_id\tsalesperson_name\twarranty_"
           "months\t"
           "warranty_provider\tfeatures\tco2_g_km\tmpg_city\tmpg_highway\t"
           "engine_displacement_l\thorsepower\ttorque_nm\tdealer_rating\t"
           "condition_notes\tservice_history\n";
  }

  // Helper to create a valid tab-separated line
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
// European Country Detection Tests
// ============================================================================

TEST_F(CarSalesAnalyzerTest, EuropeanCountryDetection) {
  EXPECT_TRUE(CarSalesAnalyzer::isEuropeanCountry("Germany"));
  EXPECT_TRUE(CarSalesAnalyzer::isEuropeanCountry("France"));
  EXPECT_TRUE(CarSalesAnalyzer::isEuropeanCountry("United Kingdom"));
  EXPECT_TRUE(CarSalesAnalyzer::isEuropeanCountry("Italy"));
  EXPECT_TRUE(CarSalesAnalyzer::isEuropeanCountry("Spain"));
  EXPECT_TRUE(CarSalesAnalyzer::isEuropeanCountry("Netherlands"));
  EXPECT_TRUE(CarSalesAnalyzer::isEuropeanCountry("Poland"));
  EXPECT_TRUE(CarSalesAnalyzer::isEuropeanCountry("Sweden"));
}

TEST_F(CarSalesAnalyzerTest, NonEuropeanCountryDetection) {
  EXPECT_FALSE(CarSalesAnalyzer::isEuropeanCountry("China"));
  EXPECT_FALSE(CarSalesAnalyzer::isEuropeanCountry("USA"));
  EXPECT_FALSE(CarSalesAnalyzer::isEuropeanCountry("Japan"));
  EXPECT_FALSE(CarSalesAnalyzer::isEuropeanCountry("Brazil"));
  EXPECT_FALSE(CarSalesAnalyzer::isEuropeanCountry("Australia"));
  EXPECT_FALSE(CarSalesAnalyzer::isEuropeanCountry("Canada"));
}

// ============================================================================
// Audi China 2025 Sales Tests
// ============================================================================

TEST_F(CarSalesAnalyzerTest, AudiChinaSales2025_SingleRecord) {
  std::string csv =
      createHeader() + createLine("15-01-2025", "China", "Audi", 45000) + "\n";

  auto result = analyzer->analyzeString(csv);

  EXPECT_EQ(result.audi_china_year_sales, 1); // Each row = 1 sale
}

TEST_F(CarSalesAnalyzerTest, AudiChinaSales2025_MultipleRecords) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "China", "Audi", 45000) + "\n";
  csv += createLine("20-01-2025", "China", "Audi", 52000) + "\n";
  csv += createLine("25-01-2025", "China", "Audi", 58000) + "\n";

  auto result = analyzer->analyzeString(csv);

  EXPECT_EQ(result.audi_china_year_sales, 3); // 3 sales
}

TEST_F(CarSalesAnalyzerTest, AudiChinaSales2025_IgnoreOtherYears) {
  std::string csv = createHeader();
  csv += createLine("15-01-2024", "China", "Audi", 45000) + "\n"; // 2024
  csv += createLine("20-01-2025", "China", "Audi", 52000) + "\n"; // 2025
  csv += createLine("25-01-2026", "China", "Audi", 58000) + "\n"; // 2026

  auto result = analyzer->analyzeString(csv);

  EXPECT_EQ(result.audi_china_year_sales, 1); // Only 2025 counts
}

TEST_F(CarSalesAnalyzerTest, AudiChinaSales2025_IgnoreOtherCountries) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "Germany", "Audi", 45000) + "\n";
  csv += createLine("20-01-2025", "China", "Audi", 52000) + "\n";
  csv += createLine("25-01-2025", "USA", "Audi", 58000) + "\n";

  auto result = analyzer->analyzeString(csv);

  EXPECT_EQ(result.audi_china_year_sales, 1); // Only China counts
}

TEST_F(CarSalesAnalyzerTest, AudiChinaSales2025_IgnoreOtherBrands) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "China", "BMW", 45000) + "\n";
  csv += createLine("20-01-2025", "China", "Audi", 52000) + "\n";
  csv += createLine("25-01-2025", "China", "Mercedes", 58000) + "\n";

  auto result = analyzer->analyzeString(csv);

  EXPECT_EQ(result.audi_china_year_sales, 1); // Only Audi counts
}

// ============================================================================
// BMW 2025 Total Revenue Tests
// ============================================================================

TEST_F(CarSalesAnalyzerTest, Bmw2025Revenue_SingleRecord) {
  std::string csv =
      createHeader() + createLine("15-01-2025", "Germany", "BMW", 48000) + "\n";

  auto result = analyzer->analyzeString(csv);

  EXPECT_DOUBLE_EQ(result.bmw_year_total_revenue, 48000);
}

TEST_F(CarSalesAnalyzerTest, Bmw2025Revenue_MultipleCountries) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "Germany", "BMW", 48000) + "\n";
  csv += createLine("20-01-2025", "China", "BMW", 75000) + "\n";
  csv += createLine("25-01-2025", "USA", "BMW", 55000) + "\n";

  auto result = analyzer->analyzeString(csv);

  EXPECT_DOUBLE_EQ(result.bmw_year_total_revenue,
                   178000); // 48000 + 75000 + 55000
}

TEST_F(CarSalesAnalyzerTest, Bmw2025Revenue_IgnoreOtherYears) {
  std::string csv = createHeader();
  csv += createLine("15-01-2024", "Germany", "BMW", 48000) + "\n"; // 2024
  csv += createLine("20-01-2025", "Germany", "BMW", 75000) + "\n"; // 2025
  csv += createLine("25-01-2026", "Germany", "BMW", 55000) + "\n"; // 2026

  auto result = analyzer->analyzeString(csv);

  EXPECT_DOUBLE_EQ(result.bmw_year_total_revenue, 75000); // Only 2025
}

TEST_F(CarSalesAnalyzerTest, Bmw2025Revenue_IgnoreOtherBrands) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "Germany", "Audi", 48000) + "\n";
  csv += createLine("20-01-2025", "Germany", "BMW", 75000) + "\n";
  csv += createLine("25-01-2025", "Germany", "Mercedes", 55000) + "\n";

  auto result = analyzer->analyzeString(csv);

  EXPECT_DOUBLE_EQ(result.bmw_year_total_revenue, 75000); // Only BMW
}

// ============================================================================
// BMW European Revenue Distribution Tests
// ============================================================================

TEST_F(CarSalesAnalyzerTest, BmwEuropeDistribution_SingleCountry) {
  std::string csv =
      createHeader() + createLine("15-01-2025", "Germany", "BMW", 48000) + "\n";

  auto result = analyzer->analyzeString(csv);

  ASSERT_EQ(result._bmw_europe_revenuedistribution.size(), 1);
  EXPECT_EQ(result._bmw_europe_revenuedistribution[0].first, "Germany");
  EXPECT_DOUBLE_EQ(result._bmw_europe_revenuedistribution[0].second, 48000);
}

TEST_F(CarSalesAnalyzerTest, BmwEuropeDistribution_MultipleCountries) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "Germany", "BMW", 48000) + "\n";
  csv += createLine("20-01-2025", "France", "BMW", 75000) + "\n";
  csv += createLine("25-01-2025", "Italy", "BMW", 35000) + "\n";

  auto result = analyzer->analyzeString(csv);

  ASSERT_EQ(result._bmw_europe_revenuedistribution.size(), 3);

  // Should be sorted by revenue descending
  EXPECT_EQ(result._bmw_europe_revenuedistribution[0].first, "France");
  EXPECT_DOUBLE_EQ(result._bmw_europe_revenuedistribution[0].second, 75000);

  EXPECT_EQ(result._bmw_europe_revenuedistribution[1].first, "Germany");
  EXPECT_DOUBLE_EQ(result._bmw_europe_revenuedistribution[1].second, 48000);

  EXPECT_EQ(result._bmw_europe_revenuedistribution[2].first, "Italy");
  EXPECT_DOUBLE_EQ(result._bmw_europe_revenuedistribution[2].second, 35000);
}

TEST_F(CarSalesAnalyzerTest, BmwEuropeDistribution_AggregatesSameCountry) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "Germany", "BMW", 48000) + "\n";
  csv += createLine("20-01-2025", "Germany", "BMW", 52000) + "\n";
  csv += createLine("25-01-2025", "France", "BMW", 35000) + "\n";

  auto result = analyzer->analyzeString(csv);

  ASSERT_EQ(result._bmw_europe_revenuedistribution.size(), 2);

  // Germany should be first with aggregated revenue
  EXPECT_EQ(result._bmw_europe_revenuedistribution[0].first, "Germany");
  EXPECT_DOUBLE_EQ(result._bmw_europe_revenuedistribution[0].second,
                   100000); // 48000 + 52000
}

TEST_F(CarSalesAnalyzerTest, BmwEuropeDistribution_ExcludesNonEuropean) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "Germany", "BMW", 48000) + "\n";
  csv +=
      createLine("20-01-2025", "China", "BMW", 150000) + "\n";  // Non-European
  csv += createLine("25-01-2025", "USA", "BMW", 120000) + "\n"; // Non-European

  auto result = analyzer->analyzeString(csv);

  ASSERT_EQ(result._bmw_europe_revenuedistribution.size(), 1);
  EXPECT_EQ(result._bmw_europe_revenuedistribution[0].first, "Germany");

  // Total revenue should include all countries
  EXPECT_DOUBLE_EQ(result.bmw_year_total_revenue,
                   318000); // 48000 + 150000 + 120000
}

TEST_F(CarSalesAnalyzerTest, BmwEuropeDistribution_IgnoresOtherBrands) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "Germany", "BMW", 48000) + "\n";
  csv +=
      createLine("20-01-2025", "France", "Audi", 75000) + "\n"; // Audi, not BMW
  csv += createLine("25-01-2025", "Italy", "Mercedes", 55000) +
         "\n"; // Mercedes, not BMW

  auto result = analyzer->analyzeString(csv);

  ASSERT_EQ(result._bmw_europe_revenuedistribution.size(), 1);
  EXPECT_EQ(result._bmw_europe_revenuedistribution[0].first, "Germany");
}

// ============================================================================
// Processing Statistics Tests
// ============================================================================

TEST_F(CarSalesAnalyzerTest, ProcessingStatistics) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "Germany", "BMW", 48000) + "\n";
  csv += createLine("20-01-2025", "China", "Audi", 52000) + "\n";
  csv += createLine("25-01-2025", "France", "Mercedes", 35000) + "\n";

  auto result = analyzer->analyzeString(csv);

  EXPECT_EQ(result.total_records_processed, 3);
  EXPECT_EQ(result.total_records_failed, 0);
  EXPECT_TRUE(result.analysis_complete);
}

// ============================================================================
// Reset Functionality Tests
// ============================================================================

TEST_F(CarSalesAnalyzerTest, ResetClearsAllData) {
  std::string csv = createHeader();
  csv += createLine("15-01-2025", "China", "Audi", 45000) + "\n";
  csv += createLine("20-01-2025", "Germany", "BMW", 75000) + "\n";

  analyzer->analyzeString(csv);

  EXPECT_EQ(analyzer->getAudiChinaSales2025(), 1);
  EXPECT_DOUBLE_EQ(analyzer->getBmw2025Revenue(), 75000);

  analyzer->reset();

  EXPECT_EQ(analyzer->getAudiChinaSales2025(), 0);
  EXPECT_DOUBLE_EQ(analyzer->getBmw2025Revenue(), 0.0);
  EXPECT_TRUE(analyzer->getBmwEuropeRevenueDistribution().empty());
}

// ============================================================================
// Empty Input Tests
// ============================================================================

TEST_F(CarSalesAnalyzerTest, EmptyInput) {
  auto result = analyzer->analyzeString("");

  EXPECT_EQ(result.audi_china_year_sales, 0);
  EXPECT_DOUBLE_EQ(result.bmw_year_total_revenue, 0.0);
  EXPECT_TRUE(result._bmw_europe_revenuedistribution.empty());
  EXPECT_EQ(result.total_records_processed, 0);
  EXPECT_TRUE(result.analysis_complete);
}

TEST_F(CarSalesAnalyzerTest, HeaderOnly) {
  std::string csv = createHeader();

  auto result = analyzer->analyzeString(csv);

  EXPECT_EQ(result.audi_china_year_sales, 0);
  EXPECT_DOUBLE_EQ(result.bmw_year_total_revenue, 0.0);
  EXPECT_TRUE(result._bmw_europe_revenuedistribution.empty());
  EXPECT_EQ(result.total_records_processed, 0);
  EXPECT_TRUE(result.analysis_complete);
}


