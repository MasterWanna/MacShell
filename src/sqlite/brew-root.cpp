#include <sqlite3.h>
#include <unistd.h>
#include "brew-cmd.hpp"
#include "io.hpp"

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
    format_print(root);

    return 0;
}