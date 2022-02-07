#ifndef CMDLINE_HPP
#define CMDLINE_HPP

#include "base-file.hpp"
#include <iostream>
#include <set>
#include <vector>
#include <unistd.h>
#include <sys/ioctl.h>

using std::cout;
using std::endl;
using std::set;
using std::string;
using std::vector;

set<string> readitem(const char *cmd)
{
    FILE *pipe = popen(cmd, "r");

    set<string> items;
    char buf[1024];
    while (fscanf(pipe, "%s", buf) != EOF)
    {
        items.insert(buf);
    }

    return items;
}

vector<string> readordered(const char *cmd)
{
    FILE *pipe = popen(cmd, "r");

    vector<string> items;
    char buf[1024];
    while (fscanf(pipe, "%s", buf) != EOF)
    {
        items.push_back(buf);
    }

    return items;
}

int runcmd(string cmd, bool echo = true)
{
    if (echo)
    {
        cout << cmd << endl;
    }
    return system(cmd.c_str());
}

int terminal_width()
{
    // get terminal width
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

#endif // CMDLINE_HPP
