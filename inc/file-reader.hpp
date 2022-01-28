#ifndef FILEREADER_HPP
#define FILEREADER_HPP

#include <fstream>
#include "base-file.hpp"
#include <vector>
#include <string>

using std::ifstream;
using std::string;
using std::vector;

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

#endif
