#include "sqlite.hpp"
#include "brew-cmd.hpp"
#include "constants.hpp"

set<string> installed;
set<string> root;
set<string> uninstalled;
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

int rmindex()
{
    if (uninstalled.size() == 0)
    {
        return 0;
    }
    string sql("delete from 'root-nodes' where ");
    bool start = false;
    for (string s : uninstalled)
    {
        if (start)
        {
            sql += " or ";
        }
        else
        {
            start = true;
        }
        sql += "name = '" + s + "'";
    }
#if DEV
    cout << sql << endl;
#endif
    sqlite_exec_once(brew_formula_db_path, sql);
    return 0;
}

int checkavail()
{
    for (string s : installed)
    {
        uninstalled.erase(s);
    }
    return 0;
}

int checkset(set<string> &torm)
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
    installed = allnodes();
    // Initialize
    sqlite_exec_multi(brew_formula_db_path, {{"create table if not exists 'root-nodes' (name text)"}, {"select * from 'root-nodes'", query}});
    // Copy root
    uninstalled = root;
    // Check uninstalled
    checkavail();
    // Remove uninstalled root index
    rmindex();
    // Read mapping
    mapping = readdeps();
    // Check rubbish
    checkset(root);
    // Remove rubbish
    return rmset();
}