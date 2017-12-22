#include "shuffle.h"
#include "print.h"
#include <regex>

using namespace csv;
using std::vector;
using std::string;

namespace shuffle {
    /** @file */

    void head(std::string infile, int nrow, std::vector<int> subset) {
        /** Print out the first n rows of a CSV */
        CSVReader reader(infile);
        vector<string> row;
        std::deque<vector<string>> records = {};
        std::vector<string> print_rows;
        int i = 0;

        while (reader.read_row(row)) {
            records.push_back(row);
            i++;

            if (i%nrow == 0) {
                while (!records.empty()) {
                    records.push_front(reader.get_col_names());
                    print_rows = break_table(records, i - records.size(), {}, true);
                    for (auto it = print_rows.begin(); it != print_rows.end(); ++it)
                        std::cout << *it << std::endl;
                    std::cout << std::endl
                        << "Press Enter to continue"
                        << std::endl << std::endl;
                    std::cin.get();
                }

                std::cout << std::endl
                    << "Press Enter to continue printing, or q or Ctrl + C to quit."
                    << std::endl << std::endl;
                if (std::cin.get() == 'q') {
                    reader.close();
                    break;
                }
            }
        }
    }

    void grep(std::string infile, int col, std::string match, int max_rows) {
        std::regex reg_pattern(match);
        std::smatch matches;
        const int orig_max_rows = max_rows;

        CSVReader reader(infile);
        vector<string> row;
        std::deque<vector<string>> records = {};

        while (reader.read_row(row)) {
            if (records.empty())
                records.push_back(reader.get_col_names());

            std::regex_search(row[col], matches, reg_pattern);
            if (!matches.empty()) {
                records.push_back(row);
                max_rows--;
            }

            if (max_rows == 0) {
                // print_table(records);
                std::cout << std::endl
                    << "Press Enter to continue searching, or q or Ctrl + C to quit."
                    << std::endl << std::endl;

                if (std::cin.get() == 'q') {
                    reader.close();
                    break;
                }
                max_rows = orig_max_rows;
            }
        }
    }
}