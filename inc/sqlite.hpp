#ifndef SQLITE_HPP
#define SQLITE_HPP

// use -lsqlite3 in compiling

#include <sqlite3.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

sqlite3 *sqlite_open(string dbpath)
{
    sqlite3 *sqlite;
    sqlite3_open(dbpath.c_str(), &sqlite);
    return sqlite;
}

void sqlite_exec(sqlite3 *sqlite, string sql, int (*callback)(void *, int, char **, char **) = nullptr, void *arg = nullptr, char **errmsg = nullptr)
{
    char *err = nullptr;
    sqlite3_exec(sqlite, sql.c_str(), callback, arg, &err);
    if (errmsg != nullptr)
    {
        *errmsg = err;
    }
    else
    {
        sqlite3_free(err);
    }
}

void sqlite_close(sqlite3 *sqlite)
{
    sqlite3_close(sqlite);
}

void sqlite_exec_once(string dbpath, string sql, int (*callback)(void *, int, char **, char **) = nullptr, void *arg = nullptr, char **errmsg = nullptr)
{
    sqlite3 *sqlite = sqlite_open(dbpath);
    sqlite_exec(sqlite, sql, callback, arg, errmsg);
    sqlite_close(sqlite);
}

struct sqlite_exec_multi_t
{
    string sql;
    int (*callback)(void *, int, char **, char **);
    void *arg;
    char **errmsg;
};

void sqlite_exec_multi(string dbpath, vector<sqlite_exec_multi_t> sqls)
{
    sqlite3 *sqlite = sqlite_open(dbpath);
    for (auto &s : sqls)
    {
        sqlite_exec(sqlite, s.sql, s.callback, s.arg, s.errmsg);
    }
    sqlite_close(sqlite);
}

#endif // SQLITE_HPP
