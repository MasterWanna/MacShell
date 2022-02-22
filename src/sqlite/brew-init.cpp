#include "sqlite.hpp"
#include "brew-cmd.hpp"
#include "constants.hpp"

set<string> installed;
set<string> root;
map<string, set<string>> mapping;

int addroot()
{
    if (root.size() == 0)
    {
        return 0;
    }
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
    sqlite_exec_once(brew_formula_db_path, sql);
    return 0;
}

int checkmapping()
{
    for (string s : installed)
    {
        set<string> subs = mapping[s];
        for (string sub : subs)
        {
            root.erase(sub);
        }
    }
    return 0;
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
    // Initailize
    sqlite_exec_multi(brew_formula_db_path, {{"create table if not exists 'root-nodes' (name text)"}, {"select * from 'root-nodes'", query}});
    // Read mapping
    mapping = readdeps();
    // Check root
    checkmapping();
    // Set root
    addroot();
    return 0;
}