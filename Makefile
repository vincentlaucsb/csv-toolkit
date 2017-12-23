CXX = clang++
BUILD_DIR = build
SQLITE_CPP = lib/sqlite-cpp
SQLITE3 = $(SQLITE_CPP)/lib
CFLAGS = -ldl -pthread -Wall -O3 -std=c++11

# Other Options
# TAR_FLAG = 
# TAR_FLAG = -target x86_64-darwin

# CSV Parser
CSV_SOURCES = $(wildcard lib/csv-parser/src/*.cpp)

TEST_SOURCES = $(wildcard tests/*.cpp)
TEST_SOURCES_NO_EXT = $(subst tests/,,$(subst .cpp,,$(TEST_SOURCES)))
TEST_DIR = tests

all: csv_parser test_all clean distclean

# SQLite3
build/sqlite3.o:
	mkdir -p $(BUILD_DIR)
	$(CC) -c -o build/sqlite3.o -O3 $(SQLITE3)/sqlite3.c -pthread -ldl -I$(SQLITE3)/
	
build/sqlite_cpp.o: build/sqlite3.o
	mkdir -p $(BUILD_DIR)
	$(CXX) -c -o build/sqlite_cpp.o $(CFLAGS) $(SQLITE_CPP)/src/sqlite_cpp.cpp -I$(SQLITE_CPP)/src/

# CSV Parser
build/csv_reader.o:
	mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CFLAGS) $(CSV_SOURCES)
	mv *.o $(BUILD_DIR)
	
# Shuffle
shuffle: build/sqlite_cpp.o build/csv_reader.o
	$(CXX) -o shuffle $(wildcard build/*.o) $(wildcard src/*.cpp) $(CFLAGS) $(TAR_FLAG)