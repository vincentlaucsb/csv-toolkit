/** @toolkit */

#pragma once
#include <csv_parser.hpp>
#include <sqlite_cpp.h>
#include <stdexcept>
#include <cstdio>
#include <sstream>
#include <set>
#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <string>
#include <vector>
#include <deque>
#include <math.h>
#include <unordered_map>
#include <set>

namespace toolkit {
    /** @file */
    using namespace csv;
    using CSVColumns = std::unordered_map<std::string, DataType>;

    /** @name SQLite Functions
     *  Functions built using the SQLite3 API
     */
    ///@{
    void csv_to_sql(std::string csv_file, std::string db,
        std::string table = "");
    void csv_join(std::string filename1, std::string filename2, std::string outfile,
        std::string column1 = "", std::string column2 = "");
    ///@}

    /**
     * @namespace csv_parser::helpers
     * @brief Helper functions for various parts of the main library
     */
    namespace helpers {
        /** @name Path Handling */
        ///@{
        std::vector<std::string> path_split(std::string);
        std::string get_filename_from_path(std::string path);
        ///@}
    }

    /**
     * @namespace csv_parser::sql
     * @brief Helper functions for SQL-related functionality
     */
    namespace sql {
        /** @name SQL Functions */
        ///@{
        std::string sql_sanitize(std::string);
        std::vector<std::string> sql_sanitize(std::vector<std::string>);
        std::vector<std::string> sqlite_types(std::string filename, int nrows = 50000);
        ///@}

        /** @name Dynamic SQL Generation */
        ///@{
        std::string create_table(std::string, std::string);
        std::string insert_values(std::string, std::string);
        ///@}
    }
}