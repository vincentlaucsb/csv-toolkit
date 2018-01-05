#include "str.h"
#include <algorithm>
#include <deque>
#include <list>
#include <math.h>

using std::unordered_map;
using std::list;

namespace shuffle {
    /** @file */

    PrettyPrinter::PrettyPrinter(PrettyPrinterParams params) {
        if (params.row_num >= 0) {
            print_row_num = true;
            begin_row_num = this->row_num = row_num;
        }

        if (!params.row_names.empty()) {
            row_names = params.row_names;
            for (auto it = row_names.begin(); it != row_names.end(); ++it) {
                if (it->size() + params.padding > row_name_width)
                    row_name_width = it->size() + params.padding;
            }
        }

        col_names = params.col_names;
        col_name_border = params.col_name_border;
        padding = params.padding;
    }

    PrettyPrinter& PrettyPrinter::operator<< (const std::vector<std::string> in) {
        return this->feed(in);
    }

    PrettyPrinter& PrettyPrinter::feed(const std::vector<std::string> in) {
        unformatted.push_back(in);
        return *this;
    }

    PrettyPrinter& PrettyPrinter::feed(std::deque<std::vector<std::string>> &in) {
        std::move(in.begin(), in.end(), std::back_inserter(unformatted));
        return *this;
    }

    void PrettyPrinter::move(std::deque<std::vector<std::string>> &records, size_t rows) {
        /** Helper function for format() which moves
         *  rows from unformatted to records
         */
        auto it_end = unformatted.begin() + std::min(rows, unformatted.size());
        for (auto it = unformatted.begin(); it != it_end && it != unformatted.end(); ) {
            records.push_back({});
            std::move(it->begin(), it->end(), std::back_inserter(records.back()));
            it = unformatted.erase(it);
        }

        row_num = begin_row_num;
    }

    void PrettyPrinter::handle_row_names(const size_t lines_past_header,
        const size_t rows) {
        // Account for column names when printing row names
        int col_names_offset = 0;
        if (!col_names.empty())
            col_names_offset = -2;

        // Adjust spacing for row names
        size_t row_name_spacing = row_name_width;
        if (row_name_spacing == 0)
            row_name_spacing = digits(begin_row_num + rows) + 2 + padding;

        if (print_row_num || !row_names.empty()) {
            if (!col_names.empty() && lines_past_header < 2)
                formatted.back() += std::string(row_name_spacing - 1, ' ');
            else if (!row_names.empty())
                formatted.back() += rpad_trim(row_names.at(lines_past_header
                    + col_names_offset), row_name_spacing);
            else
                formatted.back() += rpad_trim("[" + std::to_string(
                    row_num++) + "]", digits(begin_row_num + rows) + 2 + padding);
        }
    }

    bool PrettyPrinter::format() {
        /** Take unformatted strings and format them, returning False 
         *  if there are no more strings to format
         */
        if (!unformatted.empty()) {
            // Find out width of each column
            if (!col_names.empty()) // Include column nmaes
                this->unformatted.push_front(col_names);
            vector<size_t> col_widths = calc_widths(unformatted, 100, padding);
            size_t total_width = 0;
            for (auto it = col_widths.begin(); it != col_widths.end(); ++it)
                total_width += *it;

            // Print at most 50 rows at a time
            const size_t rows = std::min((size_t)50,
                (size_t)(50 / ((float)total_width / (float)100)));
            std::deque<std::vector<std::string>> records;
            this->move(records, rows);

            // Add dividers under column names
            if (!col_names.empty()) {
                records.push_front({});
                for (auto it = col_widths.begin(); it != col_widths.end(); ++it)
                    records.front().push_back(std::string(*it, col_name_border));
                records.begin()->swap(*(records.begin() + 1));
            }
            
            int lines_past_header = 0;
            for (auto row = records.begin(); row != records.end(); lines_past_header++) {
                formatted.push_back(std::string());
                auto col = row->begin();
                auto col_width = col_widths.begin();

                this->handle_row_names(lines_past_header, rows);
                for (size_t row_width = 0; row_width < 100 && col != row->end(); ++col) {
                    this->formatted.back() += rpad_trim(*col, *col_width, *col_width);
                    row_width += *col_width;
                    ++col_width;
                }

                row->erase(row->begin(), col); // Remove used strings
                if (row->empty())              // Remove empty rows
                    row = records.erase(row);
                else
                    row++;

                // Restart loop (if necessary)
                if (row == records.end() && !records.empty()) {
                    row = records.begin();
                    lines_past_header = -1;
                    formatted.push_back(std::string());              // For spacing
                    col_widths.erase(col_widths.begin(), col_width); // Already used
                    this->row_num = this->begin_row_num;             // Reset
                }
            }

            this->begin_row_num = this->row_num; // Update starting row number
        }

        return (!formatted.empty());
    }

    vector<size_t> PrettyPrinter::calc_widths(
        deque<vector<string>> &records,
        size_t max_col_width,
        size_t padding) {
        /** Given a list of string vectors representing rows to print,
         *  compute the width of each column
         *
         *  Rules
         *   - Doesn't return column widths > max_col_width
         *
         *  @param[out] padding Space between columns
         */

        vector<size_t> col_widths = {};
        size_t col_width;

        for (auto row = std::begin(records); row != std::end(records); ++row) {
            for (size_t i = 0; i < row->size(); i++) { // Loop through columns
                col_width = (*row)[i].size() + padding;
                if (col_width > max_col_width)
                    col_width = max_col_width;

                if (row == std::begin(records))
                    col_widths.push_back(col_width);
                else if (col_width > col_widths[i])   // Expand to fit big bois
                    col_widths[i] = col_width;
            }
        }

        return col_widths;
    }

    bool PrettyPrinter::print_rows() {
        /** Return False if user chooses not to print */
        this->format();
        while ((!unformatted.empty() || !formatted.empty())) {
            for (auto it = formatted.begin(); it != formatted.end(); ++it)
                std::cout << *it << std::endl;
            formatted.clear();
           
            if (!this->format()) break;
            else {
                std::cout << std::endl
                    << "Press Enter to continue printing, or q or Ctrl + C to quit."
                    << std::endl << std::endl;
                if (std::cin.get() == 'q')
                    return false;
            }
        }
        return true;
    }

    size_t digits(const size_t num) {
        /** Returns the number of digits in a number */
        if (num == 0)
            return 1;
        else
            return (size_t)log10(num);
    }

    void puts(std::string in) {
        std::puts(in.c_str());
    }

    string indent(const string in, size_t spaces) {
        /** Indent a string by specified number of spaces
         *  This function takes into account existing number of
         *  spaces
         */

        size_t preexisting_spaces = 0;

        for (; preexisting_spaces < in.size() &&
            in[preexisting_spaces] == ' ';
            preexisting_spaces++) {};

        if ((int)spaces - (int)preexisting_spaces > 0) {
            string new_str = std::string(" ", spaces - preexisting_spaces);
            new_str += in;
            return new_str;
        }
        else {
            return in;
        }
    }

    string rep(string in, const size_t n) {
        // Repeat and concatenate a string multiple times
        string new_str;

        for (size_t i = 0; i + 1 < n; i++)
            new_str += in;
        return new_str;
    }

    string rpad_trim(string in, size_t n, size_t trim) {
        /**
         * Add extra whitespace until string is n characters long
         * Also trim string if it is too long
         */
        std::string new_str = in;

        if (in.size() <= trim) {
            for (size_t i = in.size(); i + 1 < n; i++)
                new_str += " ";
        }
        else {
            new_str = in.substr(0, trim);
        }

        return new_str;
    }

    string round(const long double in) {
        /** Round a number to two decimal places */
        char buffer[100];
        if (isnan(in)) { return ""; }

        snprintf(buffer, 100, "%.2Lf", in);
        return std::string(buffer);
    }
    
    vector<string> round(vector<long double> in) {
        /**
         * Take a numeric vector and return a string vector with rounded numbers
         * Also replace NaNs with empty strings
         */
        vector<string> new_vec;
        for (auto num = std::begin(in); num != std::end(in); ++num)
            new_vec.push_back(round(*num));

        return new_vec;
    }

    void print_record(std::vector<std::string> record) {
        // Print out a single CSV row
        for (std::string field : record) {
            std::cout << rpad_trim(field, 20) << " ";
        }

        std::cout << std::endl;
    }

    deque<string> str_break(const string& str, const size_t target_size) {
        /** Insert line breaks into a string until each row is at most
         *  of width target_size
         */
        deque<string> broken;
        std::string sub;
        size_t prev_break_pos = 0;  // Where the last line break was
        size_t prev_space_pos = 0;  // Position is relative to beginning of str

        for (size_t i = 0, ilen = str.size(); i < ilen; i++) {
            if (str[i] == ' ')
                prev_space_pos = i;

            if ((i - prev_break_pos) == target_size) { // Break time
                if (prev_space_pos > 0) { // Break on the nearest space
                    sub = str.substr(prev_break_pos,
                        (prev_space_pos - prev_break_pos));

                    // Reset
                    prev_break_pos = prev_space_pos;
                    prev_space_pos = 0;
                }
                else { // No space --> break here
                    sub = str.substr(prev_break_pos, (i - prev_break_pos));
                    prev_break_pos = i;  // Reset
                }

                if (broken.empty()) { // First row
                    broken.push_back(sub);
                }
                else {
                    broken.push_back(indent(sub));
                }
            }
        }

        if ((&(str.back()) - &(str[prev_break_pos])) > 0)
            broken.push_back(indent(
                str.substr(prev_break_pos, std::string::npos)));
        return broken;
    }

    vector<string> long_table(
        deque<vector<string>> &records,
        const vector<size_t> col_widths
    ) {
        /** Like print_table() but adds line breaks to keep text from being too wide
         *
         *  @param[in]  records    Rows to reformat
         *  @param[out] col_widths Width of each column in characters.
         * 
         *  Line breaking rules:
         *   - Break if column width without breaking is > specified column width
         *   - Break on space if possible
         *     - Otherwise, break anywhere
         *   - If a line was broken into smaller lines, indent all lines after the first
         */

        vector<string> ret = { "" };
        std::unordered_map<size_t, deque<string>> saved;
        size_t current_width = col_widths.at(0);

        // Loop over rows
        for (auto it = records.begin(); it != records.end(); ++it) {
            // Before looping over this row's columns, print out 
            // saved stuff from previous rows
            while (true) { // Don't use (not saved.empty())
                // Check if all empty
                bool all_empty = true;

                for (size_t i = 0; i < col_widths.size(); i++) {
                    if (!saved[i].empty()) {
                        all_empty = false;
                        break;
                    }
                }

                if (all_empty)
                    break;

                // Loop over columns
                for (size_t i = 0; i < col_widths.size(); i++) {
                    current_width = col_widths.at(i);

                    if (saved[i].empty()) {
                        ret.back() += rpad_trim("", current_width);
                    }
                    else {
                        ret.back() += rpad_trim(saved[i].front(), current_width);
                        saved[i].pop_front();
                    }
                }

                ret.push_back("");
            }

            // Loop over columns
            for (size_t i = 0; i < it->size(); i++) {
                current_width = col_widths.at(i);

                // String too wide --> break it
                if (it->at(i).size() > current_width) {
                    deque<string> broken = str_break(it->at(i), current_width);
                    ret.back() += rpad_trim(broken.front(), current_width);
                    broken.pop_front();

                    if (!(saved[i].empty())) {
                        // Add to existing queue
                        std::move(
                            broken.begin(),
                            broken.end(),
                            saved[i].end());
                    }
                    else {
                        saved[i] = broken;
                    }
                }
                else {
                    ret.back() += rpad_trim(it->at(i), current_width);
                }
            }

            ret.push_back(""); // Allocate string for next row
        }

        records.clear();
        return ret;
    }
}