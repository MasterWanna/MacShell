#include <sqlite3.h>
#include "brew-cmd.hpp"

sqlite3 *sqlite;
string dbpath;
set<string> installed;
set<string> root;

enum
{
    UNINSTROOT,
    RMROOT,
    UNINST,
    NONE
};

int check(string formula)
{
    bool isroot = root.count(formula);
    bool isinst = installed.count(formula);
    if (isroot && isinst)
    {
        return UNINSTROOT;
    }
    if (!isroot && isinst)
    {
        return UNINST;
    }
    if (isroot && !isinst)
    {
        return RMROOT;
    }
    return NONE;
}

int query(void *unused, int argc, char **argv, char **column)
{
    // root nodes
    root.insert(*argv);
    return 0;
}

int rmroot(set<string> &roots, set<string> &fixes)
{
    if (roots.size() + fixes.size() == 0)
    {
        return 0;
    }
    string sql("delete from 'root-nodes' where ");
    bool start = false;
    for (string s : roots)
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
    for (string s : fixes)
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
    sqlite3_open(dbpath.c_str(), &sqlite);
    sqlite3_exec(sqlite, sql.c_str(), nullptr, nullptr, nullptr);
    sqlite3_close(sqlite);
    return 0;
}

int uninstall(set<string> &roots, set<string> &uninsts)
{
    if (roots.size() + uninsts.size() == 0)
    {
        cout << "Nothing to uninstall" << endl;
        return 0;
    }
    string cmd("brew uninstall --formula");
    for (string s : roots)
    {
        cmd += " " + s;
    }
    for (string s : uninsts)
    {
        cmd += " " + s;
    }
    return runcmd(cmd);
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
    // Check changes
    set<string> sets[NONE];
    for (int i = 1; i < argc; i++)
    {
        int status = check(argv[i]);
        if (status == NONE)
        {
            cout << "No such formula : " << argv[i] << endl;
            return 1;
        }
        sets[status].insert(argv[i]);
    }
    // uninstall required nodes
    int status = uninstall(sets[UNINSTROOT], sets[UNINST]);
    if (status == 0)
    {
        // unset root nodes
        rmroot(sets[UNINSTROOT], sets[RMROOT]);
    }
    return status;
}