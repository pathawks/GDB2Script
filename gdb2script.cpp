/**
 * Convert a text file of a GDB session into a timed typescript
 *
 * Author:      Pat Hawks
 * Created:     May 12, 2016
 * Source File: gdb2script.cpp
 */

#include <cstdlib>
#include <cstring>
#include <getopt.h>

#include <iostream>
#include <fstream>
#include <string>

extern char *__progname;

void usage(std::ostream &out) {
    int fail = out.rdbuf() == std::cerr.rdbuf();

    out << "Usage:" << std::endl;
    out << " " << __progname << " -t [timingfile] -o [typescript] [inputfile]" << std::endl;
    exit(fail ? EXIT_FAILURE : EXIT_SUCCESS);
}

int startsWith(const char *const &LINE, const std::string &PREFIX) {
    return !PREFIX.compare(0, PREFIX.length(), LINE, PREFIX.length());
}

int pauseAfterLine(const std::string &LINE) {
    const char *const KEYWORDS[] = {
        "c", "continue",
        "s", "step",
        0
    };
    char ** keyword = (char**)KEYWORDS;
    const std::string LINEWORD = LINE.substr(0, LINE.find_first_of(' '));

    while(*keyword) {
        if (LINEWORD.compare(*keyword) == 0)
            return 1;
        ++keyword;
    }

    return 0;
}

void gdb2script(std::ifstream &ifile, std::ofstream &sfile, std::ofstream
&tfile) {
    static const float PAUSE = 10;
    static const float TYPING = 0.1;
    static const float PROMPT = 2;
    static const float IMMEDIATE = 0.005;
    static const char *const ANSI_RESET = "\033[0m";
    static const char *const ANSI_PROMPT_COLOR = "\033[36m";
    static const char *const ANSI_INPUT_COLOR = "\033[33;1m";
    static const char *const ANSI_BREAK_COLOR = "\033[31m";
    static const char *const ANSI_BELL = "\007";
    static const std::string GDB_PROMPT = "(gdb) ";
    static const std::string BREAKPOINT = "Breakpoint ";
    static const int PROMPT_LENGTH = strlen(ANSI_PROMPT_COLOR) +
GDB_PROMPT.length() + strlen(ANSI_INPUT_COLOR) - 1;
    static const int RESET_LENGTH = strlen(ANSI_RESET);
    static const int BREAKPOINT_LENGTH = strlen(ANSI_BREAK_COLOR);
    static const int BELL_LENGTH = strlen(ANSI_BELL);


    char line[201];
    size_t i;

    tfile.setf(std::ios_base::fixed, std::ios_base::floatfield);
    tfile.precision(6);
    sfile << "Script generated with gdb2script" << std::endl;

    ifile.getline(line, 200);
    while (ifile.good()) {
        if (startsWith(line, GDB_PROMPT)) {
            int pause = pauseAfterLine(line + 6) ? PAUSE : 0;
            sfile << ANSI_PROMPT_COLOR << GDB_PROMPT << ANSI_INPUT_COLOR;
            sfile << (line + 6);
            if (pause)
                sfile << ANSI_BELL;
            sfile << ANSI_RESET << std::endl;
            tfile << IMMEDIATE << ' ' << PROMPT_LENGTH << std::endl;
            tfile << PROMPT << ' ' << 1 << std::endl;
            i = strlen(line + 6) + (pause ? BELL_LENGTH : 0) + 1;
            for (; i; --i) {
                tfile << TYPING << ' ' << 1 << std::endl;
            }
            tfile << pause + TYPING << ' ' << RESET_LENGTH << std::endl;
        } else if (startsWith(line, BREAKPOINT)) {
            sfile << ANSI_BREAK_COLOR << line << ANSI_RESET << std::endl;
            tfile << IMMEDIATE << ' ' << strlen(line) + BREAKPOINT_LENGTH +
RESET_LENGTH + 1 << std::endl;
        } else {
            sfile << line << std::endl;
            tfile << IMMEDIATE << ' ' << strlen(line) + 1 << std::endl;
        }

        ifile.getline(line, 200);
    }

    sfile << "End of File" << std::endl;
    return;
}

int main(int argc, char * const* argv) {
    const char *iname = NULL,  *sname = NULL, *tname = NULL;

    std::ifstream ifile;
    std::ofstream sfile;
    std::ofstream tfile;
    int c;

    const static struct option long_options[] = {
        {"typescript", required_argument, 0, 's'},
        {"timing",     required_argument, 0, 't'},
        {"help",       required_argument, 0, 'h'},
        {"version",    required_argument, 0, 'V'},
        {0, 0, 0, 0}
    };

    while ( (c = getopt_long(argc, argv, "s:t:hV", long_options, 0)) != -1) {
        switch (c) {
        case 's':
            sname = optarg;
            break;
        case 't':
            tname = optarg;
            break;
        case 'V':
            exit(EXIT_SUCCESS);
        case 'h':
            usage(std::cout);
            break;
        default:
            usage(std::cerr);
            break;
        }
    }
    argv += optind;

    if (!sname)
        sname = "typescript";
    if (!tname)
        tname = "typescript.timing";
    if (!iname && *argv)
        iname = *argv++;

    if (!iname)
        usage(std::cerr);

    ifile.open(iname, std::ofstream::in);
    sfile.open(sname, std::ofstream::out);
    tfile.open(tname, std::ofstream::out);

    if (ifile.fail())
        std::cerr << "Cannot open file " << iname << std::endl;
    if (sfile.fail())
        std::cerr << "Cannot open file " << sname << std::endl;
    if (tfile.fail())
        std::cerr << "Cannot open file " << tname << std::endl;

    if (ifile.good() && sfile.good() && tfile.good())
        gdb2script(ifile, sfile, tfile);

    tfile.close();
    sfile.close();
    ifile.close();
    std::cout << std::endl;
    return EXIT_SUCCESS;
}
