#include "sqlite.hpp"
#include "constants.hpp"
#include "io.hpp"

set<string> repos;
set<string> removed;

int query(void *unused, int argc, char **argv, char **column)
{
    // repos
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

int main()
{
    sqlite_exec_once(git_repo_db_path, "select path from 'local-repo'", query);
    for (string path : repos)
    {
        if (check_directory_exists(path + "/.git"))
        {
            cout << delimeter() << endl;
            cout << "Git Repo : " << path << " from ";
            cout.flush();
            runcmd("git -C " + path + " remote -v | grep fetch | awk '{print $2}'", false);
            if (runcmd("git -C " + path + " pull", false))
            {
                sleep(2);
            }
        }
        else
        {
            removed.insert(path);
        }
    }
    autoclean();
    return 0;
}
