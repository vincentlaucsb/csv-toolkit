#include "print.h"
#include <algorithm>
#include <deque>
#include <list>

using std::unordered_map;
using std::deque;
using std::list;

namespace shuffle {
    /** @file */

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

    vector<string> round(vector<long double> in) {
        /**
            * Take a numeric vector and return a string vector with rounded numbers
            * Also replace NaNs with empty strings
            */
        vector<string> new_vec;
        char buffer[100];
        string rounded;

        for (auto num = std::begin(in); num != std::end(in); ++num) {
            if (isnan(*num)) {
                new_vec.push_back("");
            }
            else {
                snprintf(buffer, 100, "%.2Lf", *num);
                rounded = std::string(buffer);
                new_vec.push_back(rounded);
            }
        }

        return new_vec;
    }

    vector<size_t> _get_col_widths(
        deque<vector<string>> &records,
        size_t max_col_width) {
        /** Given a list of string vectors representing rows to print,
        *  compute the width of each column
        *
        *  Rules
        *   - Doesn't return column widths > max_col_width
        */

        vector<size_t> col_widths = {};
        bool first_row = true;
        size_t col_width;

        for (auto row = std::begin(records); row != std::end(records); ++row) {
            // Looping through columns
            for (size_t i = 0; i < (*row).size(); i++) {
                // Get size of string (plus 3 for padding)
                col_width = (*row)[i].size() + 3;
                if (col_width > max_col_width)
                    col_width = max_col_width;

                // Set initial column widths
                if (first_row)
                    col_widths.push_back(col_width);

                // Update col_width if this field is a big boy
                else if (col_width > col_widths[i])
                    col_widths[i] = col_width;
            }

            first_row = false;
        }

        return col_widths;
    }

    void print_record(std::vector<std::string> record) {
        // Print out a single CSV row
        for (std::string field : record) {
            std::cout << rpad_trim(field, 20) << " ";
        }

        std::cout << std::endl;
    }

    vector<string> break_table(
        deque<vector<string>> &records,
        int row_num,
        vector<string> row_names,
        bool header
    ) {
        /**
         * Format a list of string vectors for printing
         * Set row_num to -1 to disable row number printing
         * Or set row_names to disable number printing
        */

        vector<string> ret = { "" };

        /* Find out width of each column */
        vector<size_t> col_widths = _get_col_widths(records, 100);
        const int orig_row_num = row_num;

        // Set size of row names column
        size_t row_name_width = 10;
        for (auto it = row_names.begin(); it != row_names.end(); ++it)
            if ((*it).size() > row_name_width)
                row_name_width = (*it).size();

        // Figure out how many rows an original row will be broken into
        // based on the fact each row is about 100 characters long
        size_t total_width = 0;
        for (auto it = col_widths.begin(); it != col_widths.end(); ++it)
            total_width += *it;

        // How many original rows we will print
        size_t rows = 100 / ((float)total_width / (float)100);
        const size_t orig_rows = rows;

        // Print out several vectors as a table
        auto row_name_p = row_names.begin();
        size_t col_width_p = 0, col_width_base = 0, temp_col_size = 0;
        size_t temp_row_width = 0; // Flag for when to break long rows

        // Store position in string vectors
        vector<vector<string>::iterator> cursor = {};
        for (auto record_p = records.begin(); record_p != records.end(); ++record_p)
            cursor.push_back((*record_p).begin());

        for (size_t current_row = 0, rlen = records.size();
            (current_row < rlen) && (rows > 0);
            current_row++) {

            // Hide row number for first row if header=true
            if (!row_names.empty()) {
                ret.back() += rpad_trim(*(row_name_p++), row_name_width);
            }
            else if (row_num >= 0) {
                if (header && (row_num == orig_row_num))
                    ret.back() += rpad_trim(" ", row_name_width);
                else
                    ret.back() += rpad_trim("[" + std::to_string(row_num) + "]", row_name_width);
                row_num++;
            }

            // Print out one row --> Break if necessary
            while (temp_row_width < 100
                && col_width_p != col_widths.size()) {
                temp_col_size = col_widths[col_width_p];

                /*
                if (header && (row_num == orig_row_num + 1))
                    std::cout << rpad_trim("----", temp_col_size) << std::endl;
                else
                */
                ret.back() += rpad_trim(*(cursor[current_row]), temp_col_size);

                temp_row_width += temp_col_size;
                ++col_width_p;         // Advance col width iterator
                ++cursor[current_row]; // Advance row iterator
            }

            // Prepare to move to next row
            col_width_p = col_width_base;
            rows--;
            temp_row_width = 0;
            ret.push_back("");

            // Check if we need to restart the loop & reset variables
            if (((current_row + 1 == rlen) || (rows - 1) == 0)
                && cursor[0] != records[0].end()) {
                if (!row_names.empty())
                    row_name_p = row_names.begin();
                ret.push_back("");
                rows = orig_rows;
                row_num = orig_row_num;
                col_width_base = col_width_p = cursor[0] - records[0].begin();
                current_row = -1;
            }
        }

        records.clear();
        return ret;
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