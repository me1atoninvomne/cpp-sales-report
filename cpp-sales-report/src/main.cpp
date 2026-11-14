// Usage: sales_report <sales.csv> [--top N]
#include "sales.hpp"

#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <sales.csv> [--top N]\n";
        return 2;
    }
    std::size_t top = 5;
    for (int i = 2; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--top") {
            try {
                top = static_cast<std::size_t>(std::stoul(argv[i + 1]));
            } catch (const std::exception&) {
                std::cerr << "--top needs a positive number\n";
                return 2;
            }
        }
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Cannot open file: " << argv[1] << '\n';
        return 1;
    }

    const auto parsed = sales::readSales(file);
    for (const auto& warning : parsed.warnings) {
        std::cerr << "Skipped " << warning << '\n';
    }
    sales::printReport(sales::buildReport(parsed.sales), top, std::cout);
    return 0;
}
