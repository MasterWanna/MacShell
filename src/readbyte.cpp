#include <getopt.h>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>

#define DEV 0
#define LONG 1

using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

static char short_options[] = "b:o:";

#if LONG
static struct option long_options[] =
    {
        {"base", required_argument, 0, 'b'},
        {"output", required_argument, 0, 'o'},
        {0, 0, 0, 0}
    };
#endif

static int base = 16;

static string outpath = "/dev/stdout";

static vector<string> files;

string format(int val, int base, int len)
{
    int length = 0;
    string str;
    while (val > 0)
    {
        int bit = val % base;
        str = (char)((bit > 9) ? 'A' + bit - 10 : '0' + bit) + str;
        val /= base;
        length++;
    }

    while (length++ < len)
    {
        str = '0' + str;
    }

    return str;
}

void read_item(string item)
{
    int len = log(128) / log(base) + 1;
    ifstream in(item);
    ofstream out(outpath);
    char c;
    while (in.read(&c, 1))
    {
#if DEV
        printf("%c", c);
#endif
        if (c == '\n' || c == '\r')
        {
            out << endl;
        }
        else
        {
            out << format(c, base, len) << " ";
        }
    }
    if (c != '\n')
    {
        out << endl;
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
        case 'b':
            base = strtol(optarg, NULL, 10);
            if (base > 36 || base < 2)
            {
                printf("%s: illegal base %d", argv[0], base);
                return 1;
            }
#if DEV
            output << "b " << base << endl;
#endif
            break;

        case 'o':
            outpath = optarg;
#if DEV
            output << "o " << outpath << endl;
#endif
            break;

        case -1:
            files.push_back(argv[optind]);
#if DEV
            output << c << " " << argv[optind] << endl;
#endif
            optind++;
            break;

        default:
            return 1;
        }
    }
    if (files.size() == 0)
    {
        files.push_back("/dev/stdin");
    }

    for (auto item : files)
    {
        read_item(item);
    }

    return 0;
}