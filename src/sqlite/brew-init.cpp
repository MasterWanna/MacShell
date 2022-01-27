#include <sqlite3.h>
#include "brew-cmd.hpp"

sqlite3 *sqlite;
string dbpath;
set<string> installed;
set<string> root;
map<string, set<string>> mapping;

void addroot()
{
    string sql("insert into 'root-nodes' values ");
    bool start = false;
    for (string s : root)
    {
        if (start)
        {
            sql += ", ";
        }
        else
        {
            start = true;
        }
        sql += "('" + s + "')";
    }
    sqlite3_open(dbpath.c_str(), &sqlite);
    sqlite3_exec(sqlite, sql.c_str(), nullptr, nullptr, nullptr);
    sqlite3_close(sqlite);
}

void checkmapping()
{
    for (string s : installed)
    {
        set<string> subs = mapping[s];
        for (string sub : subs)
        {
            root.erase(sub);
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
    root = installed = allnodes();
    // database
    dbpath = string(getenv("HOME")) + "/.config/brew-formula.db";
    // Open db
    sqlite3_open(dbpath.c_str(), &sqlite);
    // Drop
    sqlite3_exec(sqlite, "drop table if exists 'root-nodes'", nullptr, nullptr, nullptr);
    // Initialize
    sqlite3_exec(sqlite, "create table if not exists 'root-nodes' (name unique)", nullptr, nullptr, nullptr);
    // Close db
    sqlite3_close(sqlite);
    // Read mapping
    mapping = readdeps();
    // Check root
    checkmapping();
    // Set root
    addroot();
    return 0;
}