#include "shuffle.h"
#include "print.h"
#include "svg.h"

using namespace csv;
using std::deque;
using std::vector;
using std::string;

namespace SVG {
    std::string Element::to_string() {
        std::string ret = "<" + tag;

        // Set attributes
        for (auto it = attr.begin(); it != attr.end(); ++it)
            ret += " " + it->first + "=" + "\"" + it->second += "\"";
        ret += ">";

        // Necessary because I can't get virtual methods to work with
        // recursive function calls
        if (tag == "text") {
            ret += this->content;
        }
        else if (!this->children.empty()) {
            ret += "\n";
            // Recursively get strings for child elements
            for (auto it = children.begin(); it != children.end(); ++it)
                ret += "\t" + (*it)->to_string() + "\n";
        }

        return ret += "</" + tag + ">";
    }
}

namespace Graphs {
    // TO DO: Refactor so this and sqlite_types() share more common code
    vector<bool> numeric_types(std::string filename, int nrows) {
        /** Return the preferred data type for the columns of a file
        * @param[in] filename Path to CSV file
        * @param[in] nrows    Number of rows to examine
        */
        CSVStat stat(guess_delim(filename));
        stat.read_csv(filename, nrows, true);
        stat.calc(false, false, true);

        vector<bool> numeric_types;
        auto dtypes = stat.get_dtypes();
        size_t most_common_dtype = 0, max_count = 0;

        // Loop over each column
        for (auto col_it = dtypes.begin(); col_it != dtypes.end(); ++col_it) {
            most_common_dtype = 0;
            max_count = 0;

            // Loop over candidate data types
            for (size_t dtype = 0; dtype <= 3; dtype++) {
                try {
                    if ((size_t)col_it->at(dtype) > max_count) {
                        max_count = col_it->at(dtype);
                        most_common_dtype = dtype;
                    }
                }
                catch (std::out_of_range) {}
            }

            switch (most_common_dtype) {
            case 0:
            case 1:
                numeric_types.push_back(false);
                break;
            case 2:
            case 3:
                numeric_types.push_back(true);
                break;
            }
        }

        return numeric_types;
    }

    Graph::Graph(GraphOptions graph) {
        this->width = graph.width;
        this->height = graph.height;
        this->root.set_attr("width", width).set_attr("height", height);

        // Make title
        SVG::SVG title_wrapper;
        title_wrapper.set_attr("width", width)
            .set_attr("height", margin_top);

        SVG::Text title;
        title.set_attr("x", "50%").set_attr("y", "50%")
            .set_attr("style", "font-family: sans-serif; font-size: 24px;")
            .set_attr("dominant-baseline", "central")
            .set_attr("text-anchor", "middle");

        // Make x-axis label
        SVG::SVG xlab_wrapper;
        xlab_wrapper.set_attr("width", width).set_attr("height", 25)
            .set_attr("x", 0).set_attr("y", height - 25);

        SVG::Text xlab;
        xlab.set_attr("x", "50%").set_attr("y", "50%")
            .set_attr("style", "font-family: sans-serif; font-size: 16px;")
            .set_attr("dominant-baseline", "central")
            .set_attr("text-anchor", "middle");

        // Make y-axis label
        SVG::SVG ylab_wrapper;
        ylab_wrapper.set_attr("width", height).set_attr("height", 25)
            .set_attr("x", 0).set_attr("y", 0)
            .set_attr("transform", "translate(" +
                std::to_string(0) + "," + std::to_string(height) +
                ") rotate(-90)");

        SVG::Text ylab;
        ylab.set_attr("x", "50%").set_attr("y", "50%")
            .set_attr("style", "font-family: sans-serif; font-size: 16px;")
            .set_attr("dominant-baseline", "central")
            .set_attr("text-anchor", "middle");

        // So we can dynamically change text content later
        this->title = title_wrapper.add_child(title);
        this->xlab = xlab_wrapper.add_child(xlab);
        this->ylab = ylab_wrapper.add_child(ylab);
        this->root.add_child(title_wrapper, xlab_wrapper, ylab_wrapper);
    }

    void Graph::to_svg(const std::string filename) {
        /** Generate an SVG */
        std::ofstream svg_file(filename, std::ios_base::binary);
        svg_file << this->root.to_string();
        svg_file.close();
    }

    SVG::Group Graph::make_x_axis(Graph::x_lab_align align) {
        /** Generate the x-axis (lines, ticks, labels)
        *
        *  @param[out] align Specifies whether labels should be
        *                    left or center aligned wrt the bars
        */
        SVG::Group ret, ticks, tick_text;
        int temp_x1 = x1();

        ticks.set_attr("stroke-width", 1).set_attr("stroke", "#000000");
        tick_text.set_attr("style", "font-family: sans-serif; font-size: 12px;")
            .set_attr("text-anchor", "left");

        // Add tick marks
        size_t n = n_ticks + 1;
        size_t tick_offset = 0; // Used for alignment

        if (align == center) {
            n--;
            tick_offset += x_tick_space() / 2;
        }

        for (size_t i = 0; i < n; i++) {
            ticks.add_child(SVG::Line(
                temp_x1 + tick_offset, temp_x1 + tick_offset,
                y2(), y2() + tick_size));

            // Use translate() to set location rather than x, y
            // attributes so rotation works properly
            SVG::Text label(0, 0, x_tick_labels.at(i));
            label.set_attr("transform", "translate(" +
                std::to_string(temp_x1 + tick_offset) + "," +
                std::to_string(y2() + tick_size + 10) // Space label further south from ticks
                + ") rotate(75)");
            tick_text.add_child(label);
            temp_x1 += x_tick_space();  // Move tick mark location
        }

        SVG::Line x_axis(x1(), x2(), y2(), y2());
        x_axis.set_attr("stroke", "#cccccc").set_attr("stroke-width", 1);
        ret.add_child(x_axis, ticks, tick_text);
        return ret;
    }

    SVG::Group Graph::make_y_axis() {
        SVG::Group ret, ticks, tick_text;
        const double y_tick_gap = (y2() - y1()) / 5;
        int temp_y = y2(); // Used for placing tick marks & text

        ticks.set_attr("stroke-width", 1).set_attr("stroke", "#000000");
        tick_text.set_attr("style", "font-family: sans-serif;"
            "font-size: 12px;").set_attr("text-anchor", "end");

        // Add 6 y-axis tick marks starting from the bottom, moving upwards
        // Ticks are represented as tiny lines
        for (size_t i = 0; i < 6; i++) {
            ticks.add_child(SVG::Line(margin_left - 5, margin_left, temp_y, temp_y));
            tick_text.add_child(SVG::Text(margin_left - 5, temp_y, std::to_string(
                (long int)(range_min + i*(range_max - range_min) / 5))));
            temp_y -= y_tick_gap; // Move up
        }

        SVG::Line y_axis(x1(), x1(), y1(), y2());
        y_axis.set_attr("stroke", "#cccccc").set_attr("stroke-width", 1);

        ret.add_child(y_axis, ticks, tick_text);
        return ret;
    }

    BarChart::BarChart(
        const string filename, const string col_x, const string col_y,
        const std::unordered_map<string, string> options, GraphOptions options2) :
    Graph(options2) {
        //const string col_x, const string col_y, const string title) {
        /** Get all necessary information to generate bar chart */
        int col_pos_x = get_col_pos(filename, col_x);
        int col_pos_y = get_col_pos(filename, col_y);

        if (col_pos_x == -1)
            throw ColumnNotFoundError(col_x);

        if (col_pos_y == -1)
            throw ColumnNotFoundError(col_y);

        // Get x and y values for bar charts
        CSVReader reader(filename, GUESS_CSV, { (int)col_pos_x, (int)col_pos_y });
        vector<CSVField> row;
        string x_value;
        double y_value;

        while (reader.read_row(row) && x_tick_labels.size() < 20) {
            x_tick_labels.push_back(row[0].get_string());
            values.push_back(row[1].get_float());
        }

        this->n_ticks = x_tick_labels.size();

        // Find bin with the highest count & set bin labels
        range_min = 0;
        range_max = 0;

        for (size_t i = 0; i < n_ticks; i++) {
            if (this->values[i] > range_max)
                range_max = this->values[i];
            else if (this->values[i] < range_min)
                range_min = this->values[i];
        }

        /** Set up the graph */
        if (options.find("title") == options.end())
            this->title->content = "Chart for " + col_x + " vs. " + col_y;
        else
            this->title->content = options.find("title")->second;

        if (options.find("xlab") == options.end())
            this->xlab->content = col_x;
        else
            this->xlab->content = options.find("xlab")->second;

        if (options.find("ylab") == options.end())
            this->ylab->content = col_y;
        else
            this->ylab->content = options.find("ylab")->second;
    }

    void BarChart::generate() {
        this->root.add_child(make_bars(), make_x_axis(center), make_y_axis());
    }

    SVG::Group BarChart::make_bars() {
        /** Distribute bars evenly across graph canvas */
        SVG::Group bars;
        bars.set_attr("fill", "#004777");

        const int max_height = y2() - y1();
        int temp_x1 = x1();
        double bar_height;

        // Add a bar for every bin
        for (auto it = values.begin(); it != values.end(); ++it) {
            bar_height = (*it / range_max) * max_height;
            bars.add_child(SVG::Rect(temp_x1, y2() - bar_height,
                x_tick_space() - bar_spacing, bar_height));
            temp_x1 += x_tick_space();
        }

        return bars;
    }

    Histogram::Histogram(
        const string filename, const string col_name, const string title,
        const string x_lab, const string y_lab, const size_t bins, GraphOptions options) :
    BarChart(options) {
        /** Get all necessary information to generate histogram */
        this->n_ticks = bins;

        int col_pos = get_col_pos(filename, col_name);
        if (col_pos == -1)
            throw ColumnNotFoundError(col_name);

        // TODO: Make sure counts are numeric
        CSVStat stats;
        stats.calc_csv(filename);

        long double max = stats.get_maxes()[col_pos];
        long double min = stats.get_mins()[col_pos];
        long double bin_width = (max - min) / bins;
        auto counts = stats.get_counts()[col_pos];

        // Initialize bins
        for (size_t i = 0; i < bins; i++)
            this->values.push_back(0);

        // Aggregate each bin
        for (auto it = counts.begin(); it != counts.end(); ++it) {
            try {
                this->values[(std::stold(it->first) - min) / bin_width] += it->second;
            }
            catch (std::invalid_argument) {
                // TODO: Pass a warning to the user later
            }
        }

        // Find bin with the highest count & set bin labels
        range_min = 0;
        range_max = 0;

        for (size_t i = 0; i < n_ticks; i++) {
            if (this->values[i] > range_max)
                range_max = this->values[i];
            else if (this->values[i] < range_min)
                range_min = this->values[i];

            // Left-hand boundary values
            x_tick_labels.push_back(shuffle::round(min + i*bin_width));
        }

        // Add a label for the last tick mark
        x_tick_labels.push_back(shuffle::round(min + n_ticks*bin_width));

        /** Set up the graph */
        if (title.empty())
            this->title->content = "Histogram for " + col_name;
        else
            this->title->content = title;

        if (x_lab.empty())
            this->xlab->content = col_name;
        else
            this->xlab->content = x_lab;

        if (y_lab.empty())
            this->ylab->content = "Frequency";
        else
            this->ylab->content = y_lab;
    }

    void Histogram::generate() {
        this->root.add_child(make_bars(), make_x_axis(), make_y_axis());
    }

    Scatterplot::Scatterplot(
        const string filename, const string col_x, const string col_y,
        const string title, GraphOptions options) : Graph(options) {
        /** Get all necessary information to generate histogram */
        int col_pos_x = get_col_pos(filename, col_x);
        int col_pos_y = get_col_pos(filename, col_y);

        if (col_pos_x == -1)
            throw ColumnNotFoundError(col_x);
        if (col_pos_y == -1)
            throw ColumnNotFoundError(col_y);

        // TODO: Make sure counts are numeric
        CSVReader reader(filename, GUESS_CSV, { (int)col_pos_x, (int)col_pos_y });
        vector<CSVField> row;
        long double x_value, y_value;

        // Get min/max & points
        while (reader.read_row(row)) {
            x_value = row[0].get_float();
            y_value = row[1].get_float();

            if (isnan(domain_max)) {
                domain_max = x_value;
                domain_min = x_value;
                range_max = x_value;
                range_min = x_value;
            }

            if (x_value > domain_max)
                domain_max = x_value;
            else if (x_value < domain_min)
                domain_min = x_value;

            if (y_value > range_max)
                range_max = y_value;
            else if (y_value < range_min)
                range_min = y_value;

            points.push_back({ x_value, y_value });
        }

        this->n_ticks = std::min(points.size(), (size_t)20);

        // Set bin labels to left-hand boundary values
        for (size_t i = 0; i <= n_ticks; i++)
            this->x_tick_labels.push_back(shuffle::round(
                domain_min + i*(domain_max - domain_min) / n_ticks));

        if (title.empty())
            this->title->content = "Scatterplot for " + col_x + " vs. " + col_y;
        else
            this->title->content = title;

        this->xlab->content = col_x;
        this->ylab->content = col_y;
    }

    void Scatterplot::generate() {
        this->root.add_child(make_dots(), make_x_axis(), make_y_axis());
    }

    SVG::Group Scatterplot::make_dots() {
        SVG::Group dots;
        dots.set_attr("fill", "#004777");

        // Show every dot
        for (auto it = points.begin(); it != points.end(); ++it) {
            dots.add_child(SVG::Circle(
                (long double)x1() + (x2() - x1()) * ((*it)[0] - domain_min) / (domain_max - domain_min),
                (long double)y2() - (y2() - y1()) * ((*it)[1] - range_min) / (range_max - range_min),
                2));
        }

        return dots;
    }

    void Matrix::add_graph(Graph graph) {
        this->graphs.push_back(graph);
    }

    void Matrix::generate() {
        // Set final width & height of SVG
        root.set_attr("width", width_per_graph * cols);
        root.set_attr("height", height_per_graph *
            ceil((float)graphs.size())/(float)cols);

        size_t current_col = 0;
        size_t current_row = 0;

        // Adjust graphs
        for (auto it = graphs.begin(); it != graphs.end(); ++it) {
            it->root.set_attr("x", current_col * width_per_graph);
            it->root.set_attr("y", current_row * height_per_graph);
            it->width = width_per_graph;
            it->height = height_per_graph;
            it->generate();
            root.add_child(it->root);

            current_col++;
            if (current_col == cols) {
                current_row++;
                current_col = 0;
            }
        } 
    }

    void Matrix::to_svg(const std::string filename) {
        /** Generate an SVG */
        std::ofstream svg_file(filename, std::ios_base::binary);
        svg_file << this->root.to_string();
        svg_file.close();
    }

    void matrix_hist(const std::string filename, const std::string outfile) {
        /** Generate a matrix of histograms for all numeric columns in a file */
        GraphOptions matrix_options = { 500, 400 };
        Matrix hist_matrix(2);
        const vector<string> col_names = get_col_names(filename);
        vector<bool> numeric_cols = numeric_types(filename, 5000);

        for (size_t i = 0; i < numeric_cols.size(); i++) {
            if (numeric_cols[i]) {
                Histogram hist(filename, col_names[i], "", "", "", 20, matrix_options);
                hist.generate();
                hist_matrix.add_graph(hist);
            }
        }

        hist_matrix.generate();
        hist_matrix.to_svg(outfile);
    }
}