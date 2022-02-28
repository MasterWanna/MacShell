#ifndef TERMINAL_HPP
#define TERMINAL_HPP

// use -lncurses in compiling 

#include "cmdline.hpp"
#include <curses.h>
#include <term.h>

int tab_width()
{
    if (TABSIZE == 0)
    {
        FILE *file = fopen("/dev/null", "w+");
        delscreen(newterm(NULL, file, file));
        fclose(file);
    }
    return TABSIZE;
}

#endif // TERMINAL_HPP
