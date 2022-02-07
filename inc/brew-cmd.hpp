#ifndef BREWCMD_HPP
#define BREWCMD_HPP

#include "cmdline.hpp"
#include <map>

using std::map;

set<string> allnodes()
{
    return readitem("brew list --formula");
}

map<string, set<string>> readdeps()
{
    vector<string> items = readordered("brew deps --installed");

    map<string, set<string>> mapping;
    string name;
    set<string> deps;
    for (string item : items)
    {
        if (*(item.end() - 1) == ':')
        {
            if (name.length() != 0)
            {
                mapping[name] = deps;
            }
            size_t index = item.find_last_of('/');
            if (index == string::npos)
            {
                name = item.substr(0, item.length() - 1);
            }
            else
            {
                name = item.substr(index + 1, item.length() - index - 2);
            }
            deps = set<string>();
        }
        else
        {
            deps.insert(item);
        }
    }

    if (name.length() != 0)
    {
        mapping[name] = deps;
    }

#if DEV
    for (auto pair : mapping)
    {
        cout << pair.first << ": ";
        for (string s : pair.second)
        {
            cout << " " << s;
        }
        cout << endl;
    }
#endif

    return mapping;
}

int exists(string formula) 
{
    return !runcmd("brew info --formula " + formula + " &> /dev/null", false);
}

#endif // BREWCMD_HPP