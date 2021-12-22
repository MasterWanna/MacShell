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

static char short_options[] = "ULCS";

#if LONG
static struct option long_options[] =
    {
        {"upper", no_argument, 0, 'U'},
        {"lower", no_argument, 0, 'L'},
        {"capital", no_argument, 0, 'C'},
        {"sentense", no_argument, 0, 'S'},
        {0, 0, 0, 0}};
#endif

struct format_option
{
    char *str;
    char mod;
};

vector<format_option> options;

char opt = 'S';

void formatU(string str)
{
    for (char &c : str)
    {
        if (c >= 'a' && c <= 'z')
        {
            c -= 32;
        }
    }
    cout << str << " ";
}

void formatL(string str)
{
    for (char &c : str)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c += 32;
        }
    }
    cout << str << " ";
}

void formatC(string str)
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

void formatl(string str)
{
    char &c = str[0];
    if (c >= 'A' && c <= 'Z')
    {
        bool check = true;
        for (char i : str)
        {
            if (i >= 'a' && i <= 'z')
            {
                check = false;
                break;
            }
        }
        if (check)
        {
            c += 32;
        }
    }
    cout << str << " ";
}

void format(string str, int mod)
{
    if (str.find(' ') == string::npos)
    {
        switch (mod)
        {
        case 'U':
            formatU(str);
            break;

        case 'L':
            formatL(str);
            break;

        case 'C':
            formatC(str);
            break;

        case 'l':
            formatl(str);
            break;

        default:
            break;
        }
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
    int c;
#if LONG
    int any;
#endif
    while (optind < argc)
    {
#if LONG
        c = getopt_long(argc, argv, short_options, long_options, &any);
#if DEV
        cout << c << " " << long_options[any].name << endl;
#endif
#else
        c = getopt(argc, argv, short_options);
#endif
        switch (c)
        {
        case 'U':
        case 'L':
        case 'C':
        case 'S':
#if DEV
            cout << c << endl;
#endif
            opt = c;
            break;

        case -1:
#if DEV
            cout << c << " " << argv[optind] << endl;
#endif
            if (opt == 'S')
            {
                options.push_back({argv[optind], 'C'});
                opt = 'l';
            }
            else
            {
                options.push_back({argv[optind], opt});
            }
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