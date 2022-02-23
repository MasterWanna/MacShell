#ifndef IO_HPP
#define IO_HPP

#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include "cmdline.hpp"

using std::ifstream;
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

// format print for string set
void format_print(set<string> &items, string (*format)(string &) = nullptr)
{
    if (format == nullptr)
    {
        int maxlen = 0, len;
        for (string s : items)
        {
            len = s.length();
            if (len > maxlen)
            {
                maxlen = len;
            }
        }
        string str("%-" + to_string(maxlen) + "s ");
        int item_in_line = terminal_width() / (maxlen + 1);
        const char *buf = str.c_str();
        int size = 0;
        for (string s : items)
        {
            printf(buf, s.c_str());
            if (++size % item_in_line == 0)
            {
                printf("\n");
            }
        }
        if (size % item_in_line != 0)
        {
            printf("\n");
        }
    }
    else
    {
        int maxlen = 0, len;
        for (string s : items)
        {
            len = s.length();
            if (len > maxlen)
            {
                maxlen = len;
            }
        }
        string str("%" + to_string(maxlen) + "s : %s\n");
        const char *buf = str.c_str();
        for (string s : items)
        {
            printf(buf, s.c_str(), format(s).c_str());
        }
    }
}

// format print for string vector
void format_print(vector<string> &items, string (*format)(string &) = nullptr)
{
    if (format == nullptr)
    {
        int maxlen = 0, len;
        for (string s : items)
        {
            len = s.length();
            if (len > maxlen)
            {
                maxlen = len;
            }
        }
        string str("%-" + to_string(maxlen) + "s ");
        int item_in_line = terminal_width() / (maxlen + 1);
        const char *buf = str.c_str();
        int size = 0;
        for (string s : items)
        {
            printf(buf, s.c_str());
            if (++size % item_in_line == 0)
            {
                printf("\n");
            }
        }
        if (size % item_in_line != 0)
        {
            printf("\n");
        }
    }
    else
    {
        int size = items.size();
        int maxlen = 0, len;
        for (string s : items)
        {
            len = s.length();
            if (len > maxlen)
            {
                maxlen = len;
            }
        }
        string str("%" + to_string(maxlen) + "s : %s\n");
        const char *buf = str.c_str();
        for (string s : items)
        {
            printf(buf, s.c_str(), format(s).c_str());
        }
    }
}

// format print for string array, [start, end)
void format_print(string *items, int start, int end, string (*format)(string &) = nullptr)
{
    if (format == nullptr)
    {
        int maxlen = 0, len;
        for (int i = start; i < end; i++)
        {
            len = items[i].length();
            if (len > maxlen)
            {
                maxlen = len;
            }
        }
        string str("%-" + to_string(maxlen) + "s ");
        int item_in_line = terminal_width() / (maxlen + 1);
        const char *buf = str.c_str();
        int size = 0;
        for (int i = start; i < end; i++)
        {
            printf(buf, items[i].c_str());
            if (++size % item_in_line == 0)
            {
                printf("\n");
            }
        }
        if (size % item_in_line != 0)
        {
            printf("\n");
        }
    }
    else
    {
        int size = end - start;
        int maxlen = 0, len;
        for (int i = start; i < end; i++)
        {
            len = items[i].length();
            if (len > maxlen)
            {
                maxlen = len;
            }
        }
        string str("%" + to_string(maxlen) + "s : %s\n");
        const char *buf = str.c_str();
        for (int i = start; i < end; i++)
        {
            printf(buf, items[i].c_str(), format(items[i]).c_str());
        }
    }
}

// format print for char * array, [start, end)
void format_print(char **items, int start, int end, string (*format)(char *) = nullptr)
{
    if (format == nullptr)
    {
        int maxlen = 0, len;
        for (int i = start; i < end; i++)
        {
            len = strlen(items[i]);
            if (len > maxlen)
            {
                maxlen = len;
            }
        }
        string str("%-" + to_string(maxlen) + "s ");
        int item_in_line = terminal_width() / (maxlen + 1);
        const char *buf = str.c_str();
        int size = 0;
        for (int i = start; i < end; i++)
        {
            printf(buf, items[i]);
            if (++size % item_in_line == 0)
            {
                printf("\n");
            }
        }
        if (size % item_in_line != 0)
        {
            printf("\n");
        }
    }
    else
    {
        int size = end - start;
        int maxlen = 0, len;
        for (int i = start; i < end; i++)
        {
            len = strlen(items[i]);
            if (len > maxlen)
            {
                maxlen = len;
            }
        }
        string str("%" + to_string(maxlen) + "s : %s\n");
        const char *buf = str.c_str();
        for (int i = start; i < end; i++)
        {
            printf(buf, items[i], format(items[i]).c_str());
        }
    }
}

#endif // IO_HPP
