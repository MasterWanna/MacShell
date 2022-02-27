#ifndef IO_HPP
#define IO_HPP

#include <fstream>
#include <cstring>
#include <iomanip>
#include <sys/stat.h>
#include "cmdline.hpp"

using std::ifstream;
using std::left;
using std::max;
using std::setw;
using std::string;
using std::to_string;

bool check_file_exists(string file_path)
{
    struct stat sb;
    return stat(file_path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode);
}

bool check_directory_exists(string dir_path)
{
    struct stat sb;
    return stat(dir_path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode);
}

// read file by path
vector<string> readfile(const char *path)
{
    vector<string> items;
    ifstream file(path);
    string item;
    while (file >> item)
    {
        items.push_back(item);
    }
    return items;
}

// arrange items as in a table like
// TODO : clever arrangement needed
vector<size_t> arrange(vector<size_t> lens, int width)
{
    size_t max_len = 0;
    for (size_t len : lens)
    {
        max_len = max(max_len, len);
    }
    vector<size_t> arranged;
    int item_a_line = width / (max_len + 1);
    for (int i = 0; i < item_a_line; i++)
    {
        arranged.push_back(max_len);
    }
    return arranged;
}

// format print for string set
void format_print(set<string> &items, string (*format)(string &) = nullptr)
{
    if (format == nullptr)
    {
        size_t maxlen = 0, len;
        vector<size_t> lens;
        for (string s : items)
        {
            len = s.length();
            maxlen = max(maxlen, len);
            lens.push_back(len);
        }
        vector<size_t> arranged = arrange(lens, terminal_width());
        int i = 0, items_a_line = arranged.size();
        for (string s : items)
        {
            cout << setw(arranged[i++ % items_a_line]) << left << s << " ";
            if (i % items_a_line == 0)
            {
                cout << endl;
            }
        }
        if (i % items_a_line != 0)
        {
            cout << endl;
        }
    }
    else
    {
        size_t maxlen = 0;
        for (string s : items)
        {
            maxlen = max(maxlen, s.length());
        }
        for (string s : items)
        {
            cout << setw(maxlen) << left << s << " : " << format(s) << endl;
        }
    }
}

// format print for string vector
void format_print(vector<string> &items, string (*format)(string &) = nullptr)
{
    if (format == nullptr)
    {
        size_t maxlen = 0, len;
        vector<size_t> lens;
        for (string s : items)
        {
            len = s.length();
            maxlen = max(maxlen, len);
            lens.push_back(len);
        }
        vector<size_t> arranged = arrange(lens, terminal_width());
        int i = 0, items_a_line = arranged.size();
        for (string s : items)
        {
            cout << setw(arranged[i++ % items_a_line]) << left << s << " ";
            if (i % items_a_line == 0)
            {
                cout << endl;
            }
        }
        if (i % items_a_line != 0)
        {
            cout << endl;
        }
    }
    else
    {
        size_t maxlen = 0;
        for (string s : items)
        {
            maxlen = max(maxlen, s.length());
        }
        for (string s : items)
        {
            cout << setw(maxlen) << left << s << " : " << format(s) << endl;
        }
    }
}

// format print for string array, [start, end)
void format_print(string *items, int start, int end, string (*format)(string &) = nullptr)
{
    if (format == nullptr)
    {
        size_t maxlen = 0, len;
        vector<size_t> lens;
        for (int i = start; i < end; i++)
        {
            len = items[i].length();
            maxlen = max(maxlen, len);
            lens.push_back(len);
        }
        vector<size_t> arranged = arrange(lens, terminal_width());
        int i = 0, items_a_line = arranged.size();
        for (int i = start; i < end; i++)
        {
            cout << setw(arranged[(i - start) % items_a_line]) << left << items[i] << " ";
            if (i % items_a_line == 0)
            {
                cout << endl;
            }
        }
        if (i % items_a_line != 0)
        {
            cout << endl;
        }
    }
    else
    {
        size_t maxlen = 0;
        for (int i = start; i < end; i++)
        {
            maxlen = max(maxlen, items[i].length());
        }
        for (int i = start; i < end; i++)
        {
            cout << setw(maxlen) << left << items[i] << " : " << format(items[i]) << endl;
        }
    }
}

// format print for char * array, [start, end)
void format_print(char **items, int start, int end, string (*format)(char *) = nullptr)
{
    if (format == nullptr)
    {
        size_t maxlen = 0, len;
        vector<size_t> lens;
        for (int i = start; i < end; i++)
        {
            len = strlen(items[i]);
            maxlen = max(maxlen, len);
            lens.push_back(len);
        }
        vector<size_t> arranged = arrange(lens, terminal_width());
        int i = 0, items_a_line = arranged.size();
        for (int i = start; i < end; i++)
        {
            cout << setw(arranged[(i - start) % items_a_line]) << left << items[i] << " ";
            if (i % items_a_line == 0)
            {
                cout << endl;
            }
        }
        if (i % items_a_line != 0)
        {
            cout << endl;
        }
    }
    else
    {
        size_t maxlen = 0;
        for (int i = start; i < end; i++)
        {
            maxlen = max(maxlen, strlen(items[i]));
        }
        for (int i = start; i < end; i++)
        {
            cout << setw(maxlen) << left << items[i] << " : " << format(items[i]) << endl;
        }
    }
}

#endif // IO_HPP
