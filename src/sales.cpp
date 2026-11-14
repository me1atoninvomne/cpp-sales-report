#include "sales.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace sales {

namespace {

std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool isValidDate(const std::string& d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') {
        return false;
    }
    for (std::size_t i : {0, 1, 2, 3, 5, 6, 8, 9}) {
        if (!std::isdigit(static_cast<unsigned char>(d[i]))) {
            return false;
        }
    }
    return true;
}

std::optional<int> parseQuantity(const std::string& text) {
    if (text.empty() || text.size() > 6) {
        return std::nullopt;
    }
    for (char c : text) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return std::nullopt;
        }
    }
    const int value = std::stoi(text);
    if (value <= 0) {
        return std::nullopt;
    }
    return value;
}

}  // namespace

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (inQuotes) {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') {
                current += '"';  // "" inside quotes = one quote character
                ++i;
            } else if (c == '"') {
                inQuotes = false;
            } else {
                current += c;
            }
        } else if (c == '"') {
            inQuotes = true;
        } else if (c == ',') {
            fields.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

std::optional<Cents> parsePrice(const std::string& raw) {
    const std::string text = trim(raw);
    if (text.empty()) {
        return std::nullopt;
    }
    const auto dot = text.find('.');
    const std::string whole = text.substr(0, dot);
    std::string fraction = dot == std::string::npos ? "" : text.substr(dot + 1);
    if (whole.empty() || whole.size() > 9 || fraction.size() > 2) {
        return std::nullopt;
    }
    for (char c : whole + fraction) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return std::nullopt;
        }
    }
    while (fraction.size() < 2) {
        fraction += '0';  // "1.5" -> 150 cents
    }
    return std::stoll(whole) * 100 + std::stoll(fraction);
}

std::string formatCents(Cents cents) {
    std::ostringstream out;
    if (cents < 0) {
        out << '-';
        cents = -cents;
    }
    out << cents / 100 << '.' << std::setw(2) << std::setfill('0') << cents % 100;
    return out.str();
}

ParseResult readSales(std::istream& in) {
    ParseResult result;
    std::string line;
    int lineNumber = 0;
    while (std::getline(in, line)) {
        ++lineNumber;
        if (lineNumber == 1 || trim(line).empty()) {
            continue;  // header or empty line
        }
        const auto f = splitCsvLine(line);
        const std::string where = "line " + std::to_string(lineNumber) + ": ";
        if (f.size() != 5) {
            result.warnings.push_back(where + "expected 5 columns, got " + std::to_string(f.size()));
            continue;
        }
        Sale sale;
        sale.date = trim(f[0]);
        sale.sku = trim(f[1]);
        sale.product = trim(f[2]);
        const auto qty = parseQuantity(trim(f[3]));
        const auto price = parsePrice(f[4]);
        if (!isValidDate(sale.date)) {
            result.warnings.push_back(where + "invalid date '" + sale.date + "'");
        } else if (sale.sku.empty()) {
            result.warnings.push_back(where + "missing SKU");
        } else if (!qty) {
            result.warnings.push_back(where + "invalid quantity '" + trim(f[3]) + "'");
        } else if (!price) {
            result.warnings.push_back(where + "invalid price '" + trim(f[4]) + "'");
        } else {
            sale.quantity = *qty;
            sale.unitPrice = *price;
            result.sales.push_back(sale);
        }
    }
    return result;
}

Report buildReport(const std::vector<Sale>& sales) {
    Report report;
    std::unordered_map<std::string, ProductStats> bySku;
    for (const auto& s : sales) {
        report.totalRevenue += s.total();
        report.totalItems += s.quantity;
        report.revenueByDay[s.date] += s.total();
        auto& stats = bySku[s.sku];
        stats.sku = s.sku;
        stats.product = s.product;
        stats.quantity += s.quantity;
        stats.revenue += s.total();
    }
    for (auto& entry : bySku) {
        report.products.push_back(entry.second);
    }
    std::sort(report.products.begin(), report.products.end(), [](const ProductStats& a, const ProductStats& b) {
        if (a.revenue != b.revenue) {
            return a.revenue > b.revenue;
        }
        return a.sku < b.sku;  // stable order for equal revenue
    });
    return report;
}

void printReport(const Report& report, std::size_t top, std::ostream& out) {
    out << "SALES REPORT\n============\n";
    out << "Total revenue: " << formatCents(report.totalRevenue) << " EUR\n";
    out << "Items sold:    " << report.totalItems << "\n\n";

    out << "Revenue by day\n";
    for (const auto& [day, cents] : report.revenueByDay) {
        out << "  " << day << "  " << std::setw(10) << std::setfill(' ') << formatCents(cents) << '\n';
    }

    out << "\nTop " << std::min(top, report.products.size()) << " products by revenue\n";
    for (std::size_t i = 0; i < report.products.size() && i < top; ++i) {
        const auto& p = report.products[i];
        out << "  " << std::setw(2) << std::setfill(' ') << (i + 1) << ". " << std::left << std::setw(28) << p.product
            << std::right << std::setw(5) << p.quantity << " pcs " << std::setw(10) << formatCents(p.revenue) << '\n';
    }
}

}  // namespace sales
