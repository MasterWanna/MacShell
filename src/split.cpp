#include <vector>
#include <iostream>
#include <getopt.h>
#include <regex>

using std::cin;
using std::cout;
using std::endl;
using std::regex;
using std::sregex_token_iterator;
using std::string;
using std::vector;

static char short_options[] = "s:";

string deli("\\s");
vector<string> items;

void match(string item, regex &reg)
{
    vector<string> v(sregex_token_iterator(item.begin(), item.end(), reg, -1),
                     sregex_token_iterator());
    for (auto i : v)
    {
        cout << i;
    }
    cout << endl;
}

int main(int argc, char **argv)
{
    int c;
    while (optind < argc)
    {
        c = getopt(argc, argv, short_options);
        switch (c)
        {
        case 's':
            deli = optarg;
            break;

        case -1:
            items.push_back(argv[optind++]);
            break;

        default:
            return 1;
        }
    }

    if (items.size() == 0)
    {
        string item;
        while (cin >> item)
        {
            items.push_back(item);
        }
    }

    regex reg("[" + deli + "]");

    for (auto &item : items)
    {
        match(item, reg);
    }

    return 0;
}
