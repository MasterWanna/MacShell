#!/usr/bin/python3

from argparse import ArgumentParser
import fcntl
import math
import os
from pathlib import Path
import re
import urllib.request as urllib
from sqlite3 import Connection
import struct
import termios
from typing import Any, Dict, Iterable, List, Tuple, Union

# computation utils


char_A = ord('A')
char_0 = ord('0')


def get_value_ignore_case(dic: Dict[str, Any], key: str, default: Any = None) -> Union[str, Any]:
    for k, v in dic.items():
        if k.lower() == key.lower():
            return v
    return default


def get_real_string_len(string: str) -> int:
    length = 0
    for ch in string:
        if '\u1000' <= ch:
            length += 2
        else:
            length += 1
    return length


def byte_max_len(base: int) -> int:
    return math.ceil(math.log(255, base))


def format_int_base(num: int, base: int) -> str:
    format_num = ""

    while num != 0:
        quotient = int(num / base)
        remainder = num % base

        if remainder > 9:
            format_num = chr(char_A + remainder - 10) + format_num
        else:
            format_num = chr(char_0 + remainder) + format_num

        num = quotient

    return format_num


# command line and terminal utils


HOME = os.getenv("HOME")
db_path = "{}/.config/shell-db.db".format(HOME)
VM_NAME_MAPPING = {
    "redhat": "Red Hat"
}

linesep = os.linesep
line_sep = ord(linesep)


def run_command_s(cmd: Union[str, List[str]], input: Union[Any, List[Any]] = None) -> int:
    return run_command(cmd, False, input)


def run_command(cmd: Union[str, List[str]], echo: bool = True, input: Union[Any, List[Any]] = None) -> int:
    if echo:
        print(cmd)

    if isinstance(cmd, list):
        cmd = "; ".join(cmd)

    if input:
        pipe = os.popen(cmd, "w")
        if isinstance(input, list):
            input = to_string(input, linesep)
        pipe.write(input)

        return pipe.close()
    else:
        return os.system(cmd)


def read_command(cmd: Union[str, List[str]]) -> Tuple[int, str]:
    if isinstance(cmd, list):
        cmd = "; ".join(cmd)

    pipe = os.popen(cmd)
    res = pipe.read()

    return (pipe.close(), res)


def read_webpage(url: str) -> str:
    try:
        with urllib.urlopen(url) as f:
            return f.read().decode("utf-8")
    except Exception as e:
        return ""


def get_filename(path: str) -> str:
    return Path(path).stem


def get_file_fullname(path: str) -> str:
    return path.split(os.sep)[-1]


def get_terminal_size():
    def ioctl_GWINSZ(fd):
        try:
            cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ, '1234'))
        except:
            return
        return cr

    cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)

    if not cr:
        try:
            fd = os.open(os.ctermid(), os.O_RDONLY)
            cr = ioctl_GWINSZ(fd)
            os.close(fd)
        except:
            pass

    if not cr:
        env = os.environ
        cr = (env.get('LINES', 25), env.get('COLUMNS', 80))

    return int(cr[0]), int(cr[1])


def align_columns(strs: Iterable[str], width: int = -1) -> str:
    items = list(strs)
    max_len = max(len(s) for s in items)

    if width <= 0:
        width = get_terminal_size()[1]

    items_line = (width + 1) // (max_len + 1)

    formatted = ""

    for i in range(len(items)):
        if (i + 1) % items_line == 0:
            formatted += items[i] + linesep
        else:
            formatted += items[i].ljust(max_len + 1)

    return formatted


def get_realpath(path: str) -> str:
    return os.path.realpath(path)


def to_string(strs: Iterable[str], splitter: str = ", ", sort: bool = False) -> str:
    if sort:
        return splitter.join(sorted(strs))
    else:
        return splitter.join(strs)


def string_expand(string: str, length: int) -> str:
    str_len = len(string)

    quotient = int(length / str_len)
    remainder = length % str_len

    return string * quotient + string[:remainder]


def get_split_line(splitter: str = '-', text: str = None, length: int = 0) -> str:
    if len(splitter) != 1:
        raise ValueError("Splitter should a single char.")

    if length <= 0:
        length = get_terminal_size()[1] + length

    if length <= 0:
        raise ValueError("Length value out of range.")

    if text is None:
        return "\r" + splitter * length
    else:
        textarr = re.split(r'\s+', text)

        if len(textarr) > 0 and len(textarr[0]) == 0:
            textarr.pop(0)
        if len(textarr) > 0 and len(textarr[-1]) == 0:
            textarr.pop(-1)

        if len(textarr) == 0:
            return get_split_line(splitter, None)

        string = to_string(textarr, " ")

        str_length = len(string)
        if str_length + 4 > length:
            raise ValueError("Text too long : " + string)

        remaining = length - str_length - 2
        left = int(remaining / 2)
        right = remaining - left

        return "\r" + splitter * left + " " + string + " " + splitter * right


def get_split_block(text: Union[str, List[str]], splitter: str = '-', length: int = 0, width: Union[int, float] = 1) -> str:
    if len(splitter) != 1:
        raise ValueError("Splitter should a single char.")

    if length <= 0:
        length = get_terminal_size()[1] + length

    if length <= 0:
        raise ValueError("Length value out of range.")

    if isinstance(text, str):
        text = [text]
    
    if isinstance(width, float):
        width = int(width * length)

    block = get_split_line(splitter, length=length) + "\n"

    for line in text:
        block += splitter * width + get_split_line(" ", line, length - width * 2)[1:] + splitter * width + "\n"

    block += get_split_line(splitter, length=length)

    return "\r" + block


def list_table(title: Union[List[str], int], args: List[List[Any]]) -> str:
    if isinstance(title, int):
        max_len = [0] * title
    else:
        max_len = [len(t) for t in title]

    for arg in args:
        for i, arg_item in enumerate(arg):
            if isinstance(arg_item, str):
                arg_item_len = len(arg_item)
            else:
                arg_item_len = len(str(arg_item))

            if arg_item_len > max_len[i]:
                max_len[i] = arg_item_len

    table = get_split_line() + "\n"

    if isinstance(title, list):
        title_str = to_string([title[i].ljust(max_len[i]) for i in range(len(max_len))], "  ") + "\n"
        table += title_str + get_split_line() + "\n"
    
    for arg in args:
        arg_str = to_string([arg[i].ljust(max_len[i]) for i in range(len(max_len))], "  ") + "\n"
        table += arg_str + get_split_line() + "\n"

    return table


# db utils


def query_all(conn: Connection, table: str) -> Tuple[List[str], List[Tuple]]:
    cursor = conn.execute("select * from '{}'".format(table))
    return [des[0] for des in cursor.description], cursor.fetchall()


# passwd


passwd_path = "{}/.config/passwd".format(HOME)


# git


git_repo_table_name = "git-repo"


# git database


def read_local_repos(conn: Connection) -> List[str]:
    cursor = conn.execute(
        "select path from '{}' order by path".format(git_repo_table_name))
    return [row[0] for row in cursor.fetchall()]


# git cmdline


def realpath_repo(repos: List[str]) -> Union[List[str], None]:
    realpath_repos = []

    for path in repos:
        realpath = get_realpath(path)
        returncode, res = read_command("git -C {} rev-parse --show-toplevel 2> /dev/null".format(realpath))
        if returncode:
            print("Git repo : " + realpath + " not found.")
            return None
        else:
            realpath_repos.append(res.strip())

    return realpath_repos


def classify_repos(conn: Connection) -> List[str]:
    local_repos = read_local_repos(conn)

    git_repo_paths = []
    git_repo_remove = []

    for path in local_repos:
        if run_command_s("git -C {} rev-parse --show-toplevel &> /dev/null".format(path)):
            git_repo_remove.append(path)
        else:
            git_repo_paths.append(path)

    if len(git_repo_remove) > 0:
        print("Remove non-existent git repo : " +
              to_string(git_repo_remove, sort=True))
        conn.executemany("delete from '{}' where path = ?".format(git_repo_table_name),
                         [(path,) for path in git_repo_remove])
        conn.commit()

    return git_repo_paths


if __name__ == "__main__":
    parser = ArgumentParser(description="MacShell Utils")

    parser.parse_args()

    print("{} provides varaible and functions for MacShell.".format(get_file_fullname(__file__)))
