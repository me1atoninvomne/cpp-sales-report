// Minimal test runner without external libraries: each CHECK prints a message on failure.
#include "sales.hpp"

#include <iostream>
#include <sstream>

namespace {
int failures = 0;
int checks = 0;

#define CHECK(cond)                                                                 \
    do {                                                                            \
        ++checks;                                                                   \
        if (!(cond)) {                                                              \
            ++failures;                                                             \
            std::cerr << __FILE__ << ":" << __LINE__ << "  FAILED: " #cond << '\n'; \
        }                                                                           \
    } while (0)

void testSplitCsvLine() {
    auto f = sales::splitCsvLine(R"(2026-01-05,EGG-10,"Eggs, free range",2,2.99)");
    CHECK(f.size() == 5);
    CHECK(f[2] == "Eggs, free range");
    auto q = sales::splitCsvLine(R"(a,"say ""hi""",c)");
    CHECK(q[1] == "say \"hi\"");
    CHECK(sales::splitCsvLine("").size() == 1);
}

void testParsePrice() {
    CHECK(sales::parsePrice("1.19") == 119);
    CHECK(sales::parsePrice("1.5") == 150);
    CHECK(sales::parsePrice("3") == 300);
    CHECK(sales::parsePrice(" 0.05 ") == 5);
    CHECK(!sales::parsePrice("1.999"));
    CHECK(!sales::parsePrice("-1"));
    CHECK(!sales::parsePrice("abc"));
    CHECK(!sales::parsePrice(""));
    CHECK(!sales::parsePrice(".5"));
}

void testFormatCents() {
    CHECK(sales::formatCents(0) == "0.00");
    CHECK(sales::formatCents(5) == "0.05");
    CHECK(sales::formatCents(123456) == "1234.56");
}

void testReadSalesSkipsInvalidLines() {
    std::istringstream csv(
        "date,sku,product,quantity,unit_price\n"
        "2026-01-05,MILK-1L,Milk 1L,3,1.19\n"
        "2026-01-05,BREAD,Bread,0,2.49\n"        // quantity 0
        "05.01.2026,BREAD,Bread,1,2.49\n"        // wrong date format
        "2026-01-06,APPLE,Apples,2\n"            // missing column
        "\n"
        "2026-01-06,APPLE,Apples,2,2.99\n");
    auto r = sales::readSales(csv);
    CHECK(r.sales.size() == 2);
    CHECK(r.warnings.size() == 3);
    CHECK(r.warnings[0].find("line 3") != std::string::npos);
    CHECK(r.sales[0].unitPrice == 119);
}

void testBuildReport() {
    std::vector<sales::Sale> data = {
        {"2026-01-06", "MILK", "Milk", 2, 119},
        {"2026-01-05", "MILK", "Milk", 1, 119},
        {"2026-01-05", "COFFEE", "Coffee", 1, 549},
    };
    auto rep = sales::buildReport(data);
    CHECK(rep.totalRevenue == 2 * 119 + 119 + 549);
    CHECK(rep.totalItems == 4);
    CHECK(rep.revenueByDay.size() == 2);
    CHECK(rep.revenueByDay.begin()->first == "2026-01-05");  // days are sorted
    CHECK(rep.products.size() == 2);
    CHECK(rep.products[0].sku == "COFFEE");                   // 5.49 > 3.57
    CHECK(rep.products[1].quantity == 3);
}

void testPrintReportRespectsTop() {
    std::vector<sales::Sale> data = {
        {"2026-01-05", "A", "Alpha", 1, 300},
        {"2026-01-05", "B", "Beta", 1, 200},
        {"2026-01-05", "C", "Gamma", 1, 100},
    };
    std::ostringstream out;
    sales::printReport(sales::buildReport(data), 2, out);
    const std::string text = out.str();
    CHECK(text.find("Total revenue: 6.00 EUR") != std::string::npos);
    CHECK(text.find("Alpha") != std::string::npos);
    CHECK(text.find("Gamma") == std::string::npos);
}
}  // namespace

int main() {
    testSplitCsvLine();
    testParsePrice();
    testFormatCents();
    testReadSalesSkipsInvalidLines();
    testBuildReport();
    testPrintReportRespectsTop();
    std::cout << (checks - failures) << "/" << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
