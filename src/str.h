/**
 * String formatting functions
 */

#include <iostream>
#include <deque>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <math.h>

using std::deque;
using std::vector;
using std::string;
using std::unordered_map;
using std::list;

namespace shuffle {
    /** @file */

    struct PrettyPrinterParams {
        int row_num = -1;
        std::vector<std::string> col_names = {};
        std::vector<std::string> row_names = {};
        size_t padding = 4;
        char col_name_border = '=';
    };
    const PrettyPrinterParams DEFAULT_PRETTY_PRINT;

    class PrettyPrinter {
    public:
        PrettyPrinter(PrettyPrinterParams params=DEFAULT_PRETTY_PRINT);
        inline PrettyPrinter(std::deque<std::vector<std::string>>& in) {
            feed(in);
        };

        static std::vector<size_t> calc_widths(std::deque<std::vector<std::string>>&,
            const size_t max_col_width, const size_t padding=4);

        bool format();
        bool print_rows();

        PrettyPrinter& feed(const std::vector<std::string>);
        PrettyPrinter& feed(std::deque<std::vector<std::string>>&);
        PrettyPrinter& operator<<(const std::vector<std::string>);
    private:
        void move(std::deque<std::vector<std::string>>&, size_t);
        void handle_row_names(const size_t, const size_t);

        std::deque<std::vector<std::string>> unformatted;
        std::vector<std::string> formatted; /** Each string represents one row */
        std::vector<std::string> col_names;
        std::vector<std::string> row_names;

        char col_name_border;
        size_t row_name_width = 0;
        bool print_row_num = false;
        size_t begin_row_num = 0;
        size_t row_num = 0;
        size_t padding = 0; /** Space between columns */
    };

    /** @name String Formatting Functions
    */
    ///@{
    string rep(string in, const size_t n);
    string indent(string in, size_t spaces=2);
    string rpad_trim(string in, size_t n = 20, size_t trim = 80);
    string round(const long double in);
    vector<string> round(vector<long double> in);
    ///@}

    /** @name Pretty Printing Functions
     */
    ///@{
    size_t digits(const size_t num);
    void puts(std::string in);
    void print_record(std::vector<std::string> record);
    vector<string> long_table(std::deque<vector<string>>&, const vector<size_t>);
    ///@}

    template<typename T>
    inline vector<string> to_string(vector<T> record) {
        // Convert a vector of non-strings to a vector<string>
        vector<string> ret_vec = {};
        for (auto item : record) {
            ret_vec.push_back(std::to_string(item));
        }
        return ret_vec;
    }

    template<typename T>
    inline void _min_val_to_front(list<T>& seq) {
        /** Move the unordered_mapped element with the smallest value to the front */
        auto min_p = seq.begin();
        for (auto it = seq.begin(); it != seq.end(); ++it)
            if ((*it)->second < (*min_p)->second)
                min_p = it;

        // Swap
        if (min_p != seq.begin()) {
            auto temp = *min_p;
            seq.erase(min_p);
            seq.push_front(temp);
        }
    }

    template<typename T1, typename T2>
    inline unordered_map<T1, T2> top_n_values(unordered_map<T1, T2> in, size_t n) {
        /** Return a unordered_map with only the top n values */
        list<typename unordered_map<T1, T2>::iterator> top_n; // Ptrs to top values

        // Initialize with first n values
        auto it = in.begin();
        for (; (it != in.end()) && (top_n.size() < n); ++it)
            top_n.push_back(it);

        _min_val_to_front(top_n);  // Keep smallest value at front

        // Loop through unordered_map
        for (; it != in.end(); ++it) {
            if (it->second > (top_n.front())->second) {
                top_n.pop_front();      // Swap values
                top_n.push_front(it);
                _min_val_to_front(top_n);
            }
        }

        unordered_map<T1, T2> new_unordered_map;
        for (auto it = top_n.begin(); it != top_n.end(); ++it)
            new_unordered_map[(*it)->first] = in[(*it)->first];

        return new_unordered_map;
    }
}