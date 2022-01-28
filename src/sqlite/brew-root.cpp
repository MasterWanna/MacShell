#include <sqlite3.h>
#include <unistd.h>
#include "brew-cmd.hpp"

using std::to_string;

string dbpath;
sqlite3 *sqlite;
set<string> root;

int query(void *unused, int argc, char **argv, char **column)
{
    // root nodes
    root.insert(*argv);
    return 0;
}

int main(int argc, char **argv)
{
    // database
    dbpath = string(getenv("HOME")) + "/.config/brew-formula.db";
    // Open db
    sqlite3_open(dbpath.c_str(), &sqlite);
    // Initialize
    sqlite3_exec(sqlite, "create table if not exists 'root-nodes' (name unique)", nullptr, nullptr, nullptr);
    // Select all root nodes
    sqlite3_exec(sqlite, "select * from 'root-nodes'", query, nullptr, nullptr);
    // Close db
    sqlite3_close(sqlite);
    // Print everything in root
    int size = root.size();
    // get max length in root
    int maxlen = 0;
    for (string s : root)
    {
        if (s.length() > maxlen)
        {
            maxlen = s.length();
        }
    }
    string str("%-" + to_string(maxlen) + "s ");
    int item_in_line = terminal_width() / (maxlen + 1);
    const char *buf = str.c_str();
    size = 0;
    for (string s : root)
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

    return 0;
}