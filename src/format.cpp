#include <getopt.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#define DEV 0
#define LONG 1

using std::cin;
using std::cout;
using std::endl;
using std::istringstream;
using std::string;
using std::vector;

static char short_options[] = "ULulS";

#if LONG
static struct option long_options[] =
    {
        {"upper", no_argument, 0, 'U'},
        {"lower", no_argument, 0, 'L'},
        {"upper-capatial", no_argument, 0, 'u'},
        {"lower-capatial", no_argument, 0, 'l'},
        {"sentence", no_argument, 0, 'S'},
        {0, 0, 0, 0}};
#endif

vector<char *> options;

char mod = 'S';

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

void formatu(string str)
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

void format(string str)
{
    istringstream in(str);
    string item;
    if (mod == 'S')
    {
        mod = 'l';
        in >> item;
        formatu(item);
        while (in >> item)
        {
            formatl(item);
        }
    }
    else
    {
        void (*format_func)(string);
        switch (mod)
        {
        case 'U':
            format_func = formatU;
            break;

        case 'L':
            format_func = formatL;
            break;

        case 'u':
            format_func = formatu;
            break;

        case 'l':
            format_func = formatl;
            break;

        default:
            break;
        }
        while (in >> item)
        {
            format_func(item);
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
        case 'u':
        case 'l':
        case 'S':
#if DEV
            cout << c << endl;
#endif
            mod = c;
            break;

        case -1:
#if DEV
            cout << c << " " << argv[optind] << endl;
#endif
            options.push_back(argv[optind]);
            optind++;
            break;

        default:
            return 1;
        }
    }
    if (options.size() == 0)
    {
        string str;
        while (cin >> str)
        {
            format(str);
        }
    }
    else
    {
        for (char *option : options)
        {
            format(option);
        }
    }
    cout << endl;

    return 0;
}