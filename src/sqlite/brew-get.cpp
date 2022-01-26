#include <sqlite3.h>
#include "brew-cmd.hpp"

sqlite3 *sqlite;
string dbpath;
set<string> installed;
set<string> root;

enum
{
    OK,
    SETROOT,
    FIXINST,
    NONE,
    ERROR
};

int check(string formula)
{
    bool isroot = root.count(formula);
    bool isinst = installed.count(formula);
    if (isroot && isinst)
    {
        return OK;
    }
    if (!isroot && isinst)
    {
        return SETROOT;
    }
    if (isroot && !isinst)
    {
        return FIXINST;
    }
    if (runcmd("brew info --formula " + formula + " &> /dev/null"))
    {
        return ERROR;
    }
    return NONE;
}

int query(void *unused, int argc, char **argv, char **column)
{
    // root nodes
    root.insert(*argv);
    return 0;
}

int setroot(set<string> &subs, set<string> &insts)
{
    string sql("insert into 'root-nodes' values ");
    bool start = false;
    for (string s : subs)
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
    for (string s : insts)
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
    return 0;
}

int install(set<string> &fix, set<string> &insts)
{
    if (fix.size() + insts.size() == 0)
    {
        cout << "Nothing to install" << endl;
        return 0;
    }
    string cmd("brew install --formula");
    for (string s : fix)
    {
        cmd += " " + s;
    }
    for (string s : insts)
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
    set<string> sets[ERROR];
    for (int i = 1; i < argc; i++)
    {
        int status = check(argv[i]);
        if (status == ERROR)
        {
            cout << "No such formula : " << argv[i] << endl;
            return 1;
        }
        sets[status].insert(argv[i]);
    }
    // install non-installed nodes
    int status = install(sets[FIXINST], sets[NONE]);
    if (status == 0)
    {
        // set sub and non-installed to root nodes
        setroot(sets[SETROOT], sets[NONE]);
    }
    return status;
}