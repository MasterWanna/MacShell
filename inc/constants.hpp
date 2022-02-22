#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

using std::string;

string brew_formula_db_path(getenv("HOME") + string("/.config/brew-formula.db"));

#endif // CONSTANTS_HPP