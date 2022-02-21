#include <iostream>
#include <math.h>
#include <cstring>
#include "io.hpp"

using std::to_string;

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        format_print(argv, 1, argc, [](char *s)
                     { return to_string(strlen(s)); });
    }
    else
    {
        vector<string> items = readfile("/dev/stdin");
        format_print(items, [](string &s)
                     { return to_string(s.length()); });
    }

    return 0;
}