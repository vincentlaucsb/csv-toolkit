#include <csv_parser.hpp>
#include <string>
#include <sstream>

namespace toolkit {
    struct PGOptions {
        std::string table_name;
        size_t skiplines;
    };

    const PGOptions DEFAULT_PG = {
        "",
        0
    };

    template<typename OutputStream>
    void csv_to_postgres(const std::string& in, OutputStream& out, const PGOptions& opts = DEFAULT_PG) {
        // Convert a CSV file to a Postgres dump file
        csv::CSVReader reader(in);
        csv::StatOptions stat_options = { opts.skiplines };
        auto dtypes = csv::csv_data_types(in, stat_options);
        size_t skiplines = opts.skiplines;

        std::string table_name = opts.table_name;
        if (table_name.empty())
            table_name = in;

        // Generate CREATE TABLE statement
        out << "CREATE TABLE IF NOT EXISTS \"" << table_name << "\" (" << std::endl;

        size_t i = 0;
        for (auto& name: reader.get_col_names()) {
            auto& type = dtypes[name];
            std::string type_name;

            switch (type) {
            case csv::CSV_DOUBLE:
                type_name = "double precision";
                break;
            case csv::CSV_LONG_LONG_INT:
            case csv::CSV_LONG_INT:
            case csv::CSV_INT:
                type_name = "bigint";
                break;
            default:
                type_name = "text";
            }

            out << "\t\"" << name << "\" " << type_name;

            if (i + 1 < dtypes.size()) out << ",";
            out << std::endl;

            i++;
        }

        out << ");" << std::endl;

        // Generate COPY statement
        out << "COPY \"" << table_name << "\" FROM stdin;" << std::endl;

        // Copy CSV data
        // TODO: What to do with embedded "\t" in fields?
        for (auto& row : reader) {
            if (skiplines) {
                skiplines--;
            }
            else {
                for (size_t j = 0; j < row.size(); j++) {
                    out << row[j].get<>();
                    if (j + 1 < row.size()) out << "\t";
                    else out << "\n";
                }
            }
        }

        out << "\\." << std::endl;
    }
}