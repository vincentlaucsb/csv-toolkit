# shuffle
Shuffle is a command line tool for working with and transforming delimiter-separated values (DSV) files, such as CSV (RFC 4180), tab-delimited TXT, and so on.

![Screenshot of shuffle in action](https://raw.githubusercontent.com/vincentlaucsb/shuffle/master/screenshots/pretty-print.png)
(Shuffle can correctly guess most common delimiters. [The file above](https://github.com/vincentlaucsb/csv-data/blob/cd3d01bf60998ca7cf27759404225f60eb23564d/real_data/2009PowerStatus.txt) is pipe-delimited.)

## Features
A full list of features can be listed by typing `shuffle` in the terminal:
 * Pretty printing
 * Joining, merging, and reordering/subsetting
 * Calculating statistics and frequency counts for each column
 * Converting files to JSON and SQLite databases (with type-casting!)

## Why Shuffle?
Because life is short and RAM isn't cheap. Unlike many other tools, Shuffle is designed for speed and memory efficiency and can take advantage of multi-core processors.

For example, on my machine it takes Python's `pandas` about 32 seconds to convert a 150MB comma-separated TXT file to a SQLite3 database, compared to 12 seconds for Shuffle. It also does this with at most 50MB of memory, whereas `pandas` eats your RAM for breakfast, lunch, and dinner.

Similarly, it takes Shuffle just under 3 seconds to generate summary statistics and frequency counts [for this 80MB CSV](https://github.com/vincentlaucsb/csv-data/blob/cd3d01bf60998ca7cf27759404225f60eb23564d/real_data/2015_StateDepartment.csv). On the other hand, `CSVKit`--a popular Python package, still hasn't finished running even after 4 minutes.
