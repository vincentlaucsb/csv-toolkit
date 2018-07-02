cmake_minimum_required(VERSION 3.9)
project(shuffle)

set(CMAKE_CXX_STANDARD 17)
include(${CMAKE_SOURCE_DIR}/include/external/csv-parser/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/include/external/sqlite-cpp/CMakeLists.txt)

if (MSVC)
else()
	set(CMAKE_CXX_FLAGS "-pthread")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-Og -g -lgcov --coverage")
endif(MSVC)

set(SOURCES
	src/csv_sql.cpp
)
set(TEST_SOURCES
	tests/catch.hpp
	tests/main.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${CMAKE_SOURCE_DIR}/tests/)

## Executables
add_executable(csv_sql src/csv_sql.cpp)
target_link_libraries(csv_sql csv sqlite_cpp)
