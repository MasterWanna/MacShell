#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

using std::string;

string db_path(getenv("HOME") + string("/.config/"));
string brew_formula_db_path(db_path + "brew-formula.db");
string git_repo_db_path(db_path + "git-repo.db");

#endif // CONSTANTS_HPP