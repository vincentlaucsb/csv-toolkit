#include "shuffle.h"
#include "print.h"
#include "svg.h"

using namespace csv;
using std::deque;
using std::vector;
using std::string;

namespace Graphs {
    void Graph::init() {
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
        ylab.content = "Blah";

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

    SVG::Group Graph::make_x_axis() {
        /** Generate the x-axis (lines, ticks, labels) */
        SVG::Group ret, ticks, tick_text;
        vector<string> x_tick_labels = shuffle::round(this->x_tick_labels);
        int temp_x1 = x1();

        ticks.set_attr("stroke-width", 1).set_attr("stroke", "#000000");
        tick_text.set_attr("style", "font-family: sans-serif; font-size: 12px;")
            .set_attr("text-anchor", "left");

        // Add tick marks
        for (size_t i = 0; i <= n_ticks; i++) {
            ticks.add_child(SVG::Line(
                temp_x1, temp_x1,               // Position tick at left hand boundary
                y2(), y2() + tick_size));

            // Use translate() to set location rather than x, y
            // attributes so rotation works properly
            SVG::Text label(0, 0, x_tick_labels.at(i));
            label.set_attr("transform", "translate(" +
                std::to_string(temp_x1) + "," +
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

    Histogram::Histogram(
        const string filename, const string col_name, const string title,
        const string x_lab, const string y_lab, const size_t bins) {
        /** Get all necessary information to generate histogram */
        this->n_ticks = bins;
        this->col_name = col_name;

        int col_pos = get_col_pos(filename, col_name);
        if (col_pos == -1)
            throw ColumnNotFoundError(col_name);

        this->init();

        // TODO: Make sure counts are numeric
        CSVStat stats;
        stats.calc_csv(filename);

        long double max = stats.get_maxes()[col_pos];
        long double min = stats.get_mins()[col_pos];
        long double bin_width = (max - min) / bins;
        auto counts = stats.get_counts()[col_pos];

        // Initialize bins
        for (size_t i = 0; i < bins; i++)
            this->bins.push_back(0);

        // Aggregate each bin
        for (auto it = counts.begin(); it != counts.end(); ++it)
            this->bins[(std::stold(it->first) - min) / bin_width] += it->second;

        // Find bin with the highest count & set bin labels
        range_min = 0;
        range_max = 0;

        for (size_t i = 0; i < n_ticks; i++) {
            if (this->bins[i] > range_max)
                range_max = this->bins[i];
            else if (this->bins[i] < range_min)
                range_min = this->bins[i];

            // Left-hand boundary values
            x_tick_labels.push_back(min + i*bin_width);
        }

        // Add a label for the last tick mark
        x_tick_labels.push_back(min + n_ticks*bin_width);

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
        
        this->root.add_child(make_bars(), make_x_axis(), make_y_axis());
    }

    SVG::Group Histogram::make_bars() {
        /** Distribute bars evenly across graph canvas */

        SVG::Group bars;
        bars.set_attr("fill", "#004777");

        const int max_height = y2() - y1();
        int temp_x1 = x1();
        double bar_height;

        // Add a bar for every bin
        for (auto it = bins.begin(); it != bins.end(); ++it) {
            bar_height = (*it / range_max) * max_height;
            bars.add_child(SVG::Rect(temp_x1, y2() - bar_height,
                x_tick_space() - bar_spacing, bar_height));
            temp_x1 += x_tick_space();
        }

        return bars;
    }

    Scatterplot::Scatterplot(
        const string filename, const string col_x, const string col_y,
        const string title) {
        this->init();

        /** Get all necessary information to generate histogram */
        this->col_name = col_name;
        this->title->content = title;
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

        // Set bin labels to left-hand boundary values
        for (size_t i = 0; i <= n_ticks; i++)
            this->x_tick_labels.push_back(domain_min + i*(domain_max - domain_min)/n_ticks);

        if (title.empty())
            this->title->content = "Scatterplot for " + col_x + " vs. " + col_y;
        else
            this->title->content = title;

        this->xlab->content = col_x;
        this->ylab->content = col_y;
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
}