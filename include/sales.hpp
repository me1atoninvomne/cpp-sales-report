#pragma once

#include <cstdint>
#include <istream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace sales {

/// Money is stored in cents (integer), never in double, so sums are always exact.
using Cents = std::int64_t;

struct Sale {
    std::string date;      // YYYY-MM-DD
    std::string sku;
    std::string product;
    int quantity = 0;
    Cents unitPrice = 0;

    Cents total() const { return unitPrice * quantity; }
};

struct ParseResult {
    std::vector<Sale> sales;
    std::vector<std::string> warnings;  // one message per skipped line
};

struct ProductStats {
    std::string sku;
    std::string product;
    int quantity = 0;
    Cents revenue = 0;
};

struct Report {
    Cents totalRevenue = 0;
    int totalItems = 0;
    std::map<std::string, Cents> revenueByDay;  // sorted by date
    std::vector<ProductStats> products;         // sorted by revenue, highest first
};

/// Splits one CSV line into fields. Supports quoted fields with commas and "" escapes.
std::vector<std::string> splitCsvLine(const std::string& line);

/// Parses a price like "1.19", "3", "0.5" into cents. Returns nothing if the text is not a valid price.
std::optional<Cents> parsePrice(const std::string& text);

/// Formats cents as "1234.50".
std::string formatCents(Cents cents);

/// Reads CSV with header: date,sku,product,quantity,unit_price. Invalid lines are skipped with a warning.
ParseResult readSales(std::istream& in);

/// Calculates totals, revenue per day and statistics per product.
Report buildReport(const std::vector<Sale>& sales);

/// Prints the report as text; shows at most `top` products.
void printReport(const Report& report, std::size_t top, std::ostream& out);

}  // namespace sales
