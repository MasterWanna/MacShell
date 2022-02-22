#include "sqlite.hpp"
#include <unistd.h>
#include "brew-cmd.hpp"
#include "io.hpp"
#include "constants.hpp"

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
    // Initialize
    sqlite_exec_multi(brew_formula_db_path, {{"create table if not exists 'root-nodes' (name text)"}, {"select * from 'root-nodes'", query}});
    // Print everything in root
    int size = root.size();
    // get max length in root
    format_print(root);

    return 0;
}