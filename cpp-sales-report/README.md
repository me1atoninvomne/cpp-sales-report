# Sales Report (C++)

A command-line tool in **C++17** that reads a shop's daily sales export (CSV) and prints a report:
total revenue, items sold, revenue per day and the best-selling products.

When I worked as a shift manager I prepared daily sales statistics in Excel. This project does the
same calculation as a small, tested C++ program.

## Example

```bash
./build/sales_report data/sales_sample.csv --top 5
```

```
Skipped line 16: invalid quantity '0'
Skipped line 23: invalid date '2026-3-06'
SALES REPORT
============
Total revenue: 401.08 EUR
Items sold:    212

Revenue by day
  2026-03-02       94.15
  2026-03-03      110.79
  2026-03-04       86.98
  2026-03-05      109.16

Top 5 products by revenue
   1. Ground coffee 250g             17 pcs      93.33
   2. Whole grain bread              31 pcs      77.19
   3. Milk 1L                        54 pcs      64.26
   4. Eggs, free range (10 pcs)      18 pcs      53.82
   5. Still water 1.5L               70 pcs      48.30
```

Input format (`date,sku,product,quantity,unit_price`):

```csv
date,sku,product,quantity,unit_price
2026-03-02,MILK-1L,Milk 1L,14,1.19
2026-03-03,EGGS-10,"Eggs, free range (10 pcs)",7,2.99
```

## Build and test

Requirements: CMake 3.16+ and a C++17 compiler (GCC, Clang or MSVC).

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Implementation notes

- **Money in cents (`int64_t`)**, not `double` — `0.1 + 0.2` problems cannot happen and totals are exact.
- **CSV parser** supports quoted fields with commas and escaped quotes (`"Eggs, free range"`, `"say ""hi"""`).
- **Bad lines do not stop the program**: each invalid line (wrong date, quantity ≤ 0, invalid price,
  wrong number of columns) is skipped and reported on `stderr` with its line number.
- `std::map` keeps days sorted; products are sorted by revenue with a stable tie-break by SKU.
- Logic is in a small library (`sales`) used by both the program and the tests; tests use a tiny
  `CHECK` macro, so no external test framework is needed.

```
include/sales.hpp     data types and function declarations
src/sales.cpp         parsing, calculations, report printing
src/main.cpp          command-line interface
tests/test_sales.cpp  unit tests
data/                 sample CSV
```

---

*Built with AI assistance (Claude) as a learning and portfolio project.*
