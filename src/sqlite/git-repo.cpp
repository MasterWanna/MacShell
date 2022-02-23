#include "sqlite.hpp"
#include "io.hpp"
#include "constants.hpp"
#include <getopt.h>

static char short_options[] = "r";

set<string> setup;
bool rm = false;
set<string> repos;
set<string> realpaths;

int query(void *unused, int argc, char **argv, char **column)
{
    // repos
    setup.insert(*argv);
    return 0;
}

void run_sql()
{
    if (realpaths.size() == 0)
    {
        return;
    }
    if (rm)
    {
        string sql("delete from 'local-repo' where ");
        bool start = false;
        for (string s : realpaths)
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
    }
    else
    {
        string sql("insert into 'local-repo' values ");
        bool start = false;
        for (string s : realpaths)
        {
            if (start)
            {
                sql += ",";
            }
            else
            {
                start = true;
            }
            sql += "('" + s + "')";
        }
#if DEV
        cout << sql << endl;
#endif
        sqlite_exec_once(git_repo_db_path, sql);
    }
}

int main(int argc, char **argv)
{
    int c;
    while (optind < argc)
    {
        c = getopt(argc, argv, short_options);
        switch (c)
        {
        case 'r':
#if DEV
            cout << "rm" << endl;
#endif
            rm = true;
            break;

        case -1:
#if DEV
            cout << argv[optind] << endl;
#endif
            repos.insert(argv[optind++]);
            break;

        default:
            return 1;
        }
    }

    if (repos.size() == 0)
    {
        cout << "Usage: git repo [-r] repo1 [repo2 ...]" << endl;
        return 1;
    }

    for (string s : repos)
    {
        realpaths.insert(realpath(s.c_str(), nullptr));
    }

    if (!rm)
    {
        sqlite_exec_once(git_repo_db_path, "select path from 'local-repo'", query);
        for (string path : realpaths)
        {
            if (!check_directory_exists(path + "/.git"))
            {
                cout << "Git Repo : " << path << " not found" << endl;
                return 1;
            }

            if (setup.find(path) != setup.end())
            {
                cout << "Git Repo : " << path << " already setup" << endl;
                return 1;
            }
        }
    }

    run_sql();

    return 0;
}
