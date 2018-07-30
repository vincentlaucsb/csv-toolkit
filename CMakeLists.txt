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

set(TEST_SOURCES
	${CMAKE_SOURCE_DIR}/tests/catch.hpp
	${CMAKE_SOURCE_DIR}/tests/main.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/include/external/)
include_directories(${CMAKE_SOURCE_DIR}/tests/)

## Executables
add_executable(csvjson include/internal/csv_json.cpp)
target_link_libraries(csvjson csv)

add_executable(csvsql include/internal/csv_sql.cpp)
target_link_libraries(csvsql csv sqlite_cpp)

add_executable(csvpg include/internal/csv_postgres.cpp)
target_link_libraries(csvpg csv)

add_executable(csvtest ${TEST_SOURCES})
target_link_libraries(csvtest csv)