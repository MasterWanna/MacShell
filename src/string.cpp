#include <getopt.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#define DEV 0
#define LONG 1

using std::cout;
using std::endl;
using std::istringstream;
using std::string;
using std::vector;

static char short_options[] = "ULC";

#if LONG
static struct option long_options[] =
    {
        {"upper", no_argument, 0, 'U'},
        {"lower", no_argument, 0, 'L'},
        {"capital", no_argument, 0, 'C'},
        {0, 0, 0, 0}};
#endif

struct format_option
{
    char *str;
    char mod;
};

vector<format_option> options;

char opt = 'C';

void format(string str, int mod)
{
    if (str.find(' ') == string::npos)
    {
        char &c = str[0];
        if (c >= 'a' && c <= 'z')
        {
            bool check = true;
            for (char i : str)
            {
                if (i >= 'A' && i <= 'Z')
                {
                    check = false;
                    break;
                }
            }
            if (check)
            {
                c -= 32;
            }
        }
        cout << str << " ";
    }
    else
    {
        istringstream in(str);
        string item;
        while (in >> item)
        {
            format(item, mod);
        }
    }
}

int main(int argc, char **argv)
{
#if DEV
    outpath = "/dev/null";
    ofstream output("/dev/stdout");
#endif
    int c;
#if LONG
    int any;
#endif
    while (optind < argc)
    {
#if LONG
        c = getopt_long(argc, argv, short_options, long_options, &any);
#if DEV
        output << c << " " << long_options[any].name << endl;
#endif
#else
        c = getopt(argc, argv, short_options);
#endif
        switch (c)
        {
        case 'U':
        case 'L':
        case 'C':
#if DEV
            output << c << endl;
#endif
            opt = c;
            break;

        case -1:
#if DEV
            output << c << " " << argv[optind] << endl;
#endif
            options.push_back({argv[optind], opt});
            optind++;
            break;

        default:
            return 1;
        }
    }
    for (format_option option : options)
    {
        format(option.str, option.mod);
    }
    cout << endl;

    return 0;
}