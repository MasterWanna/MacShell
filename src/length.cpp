#include <iostream>
#include <math.h>
#include "file-reader.hpp"

using std::to_string;

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        int len[argc];
        int maxlen = 0;
        for (int i = 1; i < argc; i++)
        {
            len[i] = strlen(argv[i]);
            if (len[i] > maxlen)
            {
                maxlen = len[i];
            }
        }
        string str("%" + to_string(maxlen) + "s : %d\n");
        const char *buf = str.c_str();
        for (int i = 1; i < argc; i++)
        {
            printf(buf, argv[i], len[i]);
        }
    }
    else
    {
        vector<string> items = readfile("/dev/stdin");
        int size = items.size();
        int len[size];
        int maxlen = 0;
        for (int i = 0; i < size; i++)
        {
            len[i] = items[i].length();
            if (len[i] > maxlen)
            {
                maxlen = len[i];
            }
        }
        string str("%" + to_string(maxlen) + "s : %d\n");
        const char *buf = str.c_str();
        for (int i = 0; i < argc; i++)
        {
            printf(buf, items[i].c_str(), len[i]);
        }
    }

    return 0;
}