using std::deque;
using std::string;
using std::unordered_map;

namespace shuffle {
    int getargs(int argc, char* argv[],
        deque<string>& args,
        deque<string>& flags,
        unordered_map<string, string>& options
    ) {
        /**
         *  Parse command line arguments
         *  Return 1 if there is a parsing error
         *
         *  Syntax:
         *  - Arguments are parsed in the order they come and are space-delimited
         *    - Arguments may be quoted and "example 1" is treated as one argument
         *  - Flags are any arguments prefixed with a hyphen
         *  - Options are any arguments in the form -option=value, and 
         *    are a special case of flags
         *
         */

        bool quote_escape = false;
        std::string this_arg;

        // Skip first argument --> it's the program name
        for (int i = 1; i < argc; i++) {
            this_arg = std::string(argv[i]);

            if (quote_escape) {
                // Quote Escape Handling
                if (this_arg.back() == '"') {
                    quote_escape = false;
                    this_arg = this_arg.substr(0, this_arg.size() - 1);
                }
                else {
                    if (i + 1 == argc) {
                        // End of arguments but no end of quote escape in sight
                        return 1;
                    }
                }

                // Append current string to previous argument string
                args.back() = args.back() + " " + this_arg;
            }
            else if (this_arg.front() == '-') {
                size_t begin = 1;
                if (this_arg[1] == '-')
                    begin++;

                // Option flag
                flags.push_back(this_arg.substr(begin, this_arg.size()));
            }
            else {
                if (this_arg[0] == '"') {
                    quote_escape = true;
                    this_arg = this_arg.substr(1); // Trim off quote
                }

                args.push_back(this_arg);
            }
        }

        // Separate option flags from other flags
        for (auto it = flags.begin(); it != flags.end();) {
            size_t sep_pos = it->find("=");
            if (sep_pos != std::string::npos) {
                // Add to options map
                options[it->substr(0, sep_pos)] = it->substr(sep_pos + 1);

                // Pop from args
                it = flags.erase(it);
            }
            else {
                ++it;
            }
        }

        return 0;
    }
}