#include "svg.h"

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