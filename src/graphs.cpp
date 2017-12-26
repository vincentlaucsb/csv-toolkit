#include "shuffle.h"
#include "svg.h"

using namespace csv;
using std::deque;
using std::vector;
using std::string;

namespace Graphs {
    void Graph::to_svg(const std::string filename) {
        /** Generate an SVG */
        std::ofstream svg_file(filename, std::ios_base::binary);
        svg_file << this->root.to_string();
        svg_file.close();
    }

    Histogram::Histogram(
        const string filename, const string col_name, const size_t bins) {
        /** Get all necessary information to generate histogram */
        this->n_ticks = bins;
        this->col_name = col_name;
        size_t col_pos = get_col_pos(filename, col_name);

        // TODO: Make sure counts are numeric
        CSVStat stats;
        stats.calc_csv(filename);

        long double max = stats.get_maxes()[col_pos];
        long double min = stats.get_mins()[col_pos];
        auto counts = stats.get_counts()[col_pos];
        long double bin_width = (max - min) / bins;

        // Initialize bins & bin_labels
        for (size_t i = 0; i < bins; i++) {
            this->bins.push_back(0);
            this->x_tick_labels.push_back("");
        }

        // Aggregate each bin
        for (auto it = counts.begin(); it != counts.end(); ++it)
            this->bins[(std::stold(it->first) - min) / bin_width] += it->second;

        // Find bin with the highest count & set bin labels
        for (size_t i = 0; i < n_ticks; i++) {
            if (this->bins[i] > range_max)
                range_max = this->bins[i];
            else if (this->bins[i] < range_min)
                range_min = this->bins[i];

            // Left-hand boundary values
            this->x_tick_labels[i] = std::to_string(min + i*bin_width);
        }

        /** Set up the graph */
        this->root.set_attr("width", width).set_attr("height", height);

        SVG::Text title(margin_left, margin_top - 20, "Histogram for " + col_name);
        title.set_attr("style", "font-family: sans-serif; font-size: 24px;");
        this->root.add_child(title,
            this->make_bars(),
            this->make_x_axis(),
            this->make_y_axis()
        );
    }

    SVG::Group Graph::make_x_axis() {
        /** Generate the x-axis (lines, ticks, labels) */
        SVG::Group ret, ticks, tick_text;
        int temp_x1 = x1();

        ticks.set_attr("stroke-width", 1).set_attr("stroke", "#000000");
        tick_text.set_attr("style", "font-family: sans-serif; font-size: 12px;")
            .set_attr("text-anchor", "left");

        // Add a bar for every bin
        for (size_t i = 0; i < n_ticks; i++) {
            ticks.add_child(SVG::Line(          // Tick Marks
                temp_x1, temp_x1,               // Position tick at left hand boundary
                y2(), y2() + tick_size));
            
            // Use translate() to set location rather than x, y
            // attributes so rotation works properly
            SVG::Text label(0, 0, x_tick_labels.at(i));
            label.set_attr("transform", "translate(" +
                std::to_string(temp_x1 + x_tick_space() / 2) + "," +
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

}