#include <csv_parser.hpp>
#include <json.hpp>
#include <string>
#include <sstream>

namespace toolkit {
    using json = nlohmann::json;

    template<typename OutputStream>
    void csv_to_json(const std::string& in, OutputStream& out) {
        /** Convert a CSV file to JSON */
        using namespace csv;
        CSVReader reader(in);
        auto col_names = reader.get_col_names();

        out << "[";
        bool first_row = true;

        for (auto& row : reader) {
            if (first_row)
                first_row = false;
            else
                out << ",\n";

            json record;
            for (auto& name : col_names) {
                DataType type = row[name].type();
                
                switch (type) {
                case CSV_DOUBLE:
                    record[name] = row[name].get<double>();
                    break;
                case CSV_LONG_LONG_INT:
                    record[name] = row[name].get<long long int>();
                    break;
                case CSV_LONG_INT:
                    record[name] = row[name].get<long int>();
                    break;
                case CSV_INT:
                    record[name] = row[name].get<int>();
                    break;
                default:
                    record[name] = row[name].get<>();
                }

            }

            out << record;
        }
        
        out << "\n]";
    }
}