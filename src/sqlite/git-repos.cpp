#include "sqlite.hpp"
#include "io.hpp"
#include "constants.hpp"
#include <limits.h>
#include <math.h>
#include <iomanip>

using std::left;
using std::max;
using std::setw;

set<string> repos;
set<string> removed;
size_t maxlen = 0;

int query(void *unused, int argc, char **argv, char **column)
{
    // repos
    maxlen = max(maxlen, strlen(*argv));
    repos.insert(*argv);
    return 0;
}

int autoclean()
{
    if (removed.size() == 0)
    {
        return 0;
    }
    string sql("delete from 'local-repo' where ");
    bool start = false;
    for (string s : removed)
    {
        if (start)
        {
            sql += " or ";
        }
        else
        {
            start = true;
        }
        sql += "path = '" + s + "'";
    }
    sqlite_exec_once(git_repo_db_path, sql);
    return 0;
}

int main(int argc, char **argv)
{
    sqlite_exec_once(git_repo_db_path, "select path from 'local-repo'", query);

    for (string path : repos)
    {
        if (check_directory_exists(path + "/.git"))
        {
            cout << setw(maxlen) << left << path << " : ";
            cout.flush();
            runcmd("git -C " + path + " remote -v | grep fetch | awk '{print $2}'", false);
        }
        else
        {
            removed.insert(path);
        }
    }

    return 0;
}
