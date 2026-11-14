cmake_minimum_required(VERSION 3.16)
project(sales_report LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Library with the logic, used by both the program and the tests
add_library(sales STATIC src/sales.cpp)
target_include_directories(sales PUBLIC include)

add_executable(sales_report src/main.cpp)
target_link_libraries(sales_report PRIVATE sales)

enable_testing()
add_executable(sales_tests tests/test_sales.cpp)
target_link_libraries(sales_tests PRIVATE sales)
add_test(NAME sales_tests COMMAND sales_tests)
