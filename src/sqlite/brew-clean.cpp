#include <sqlite3.h>
#include "brew-cmd.hpp"

sqlite3 *sqlite;
string dbpath;
set<string> installed;
set<string> root;
map<string, set<string>> mapping;

int rmset()
{
    if (installed.size() == 0)
    {
        cout << "Nothing to uninstall" << endl;
        return 0;
    }
    string cmd("brew uninstall --formula");
    for (string s : installed)
    {
        cmd += " " + s;
    }
    return runcmd(cmd);
}

void checkset(set<string> torm)
{
    for (string s : torm)
    {
        if (installed.count(s) != 0)
        {
            installed.erase(s);
            set<string> d = mapping[s];
            if (d.size() > 0)
            {
                checkset(d);
            }
        }
    }
}

int query(void *unused, int argc, char **argv, char **column)
{
    // root nodes
    root.insert(*argv);
    return 0;
}

int main(int argc, char **argv)
{
    // brew nodes
    installed = allnodes();
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
    // Read mapping
    mapping = readdeps();
    // Check rubbish
    checkset(root);
    // Remove rubbish
    return rmset();
}