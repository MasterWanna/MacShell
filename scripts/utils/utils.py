import fcntl
import math
import os
from pathlib import Path
import re
from sqlite3 import Connection
import struct
import termios
from typing import Iterable, List, Tuple, Union

# computation utils


char_A = ord('A')
char_0 = ord('0')


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

linesep = os.linesep
line_sep = ord(linesep)


def run_command_s(cmd: Union[str, List[str]]) -> int:
    return run_command(cmd, False)


def run_command(cmd: Union[str, List[str]], echo: bool = True) -> int:
    if echo:
        print(cmd)

    if isinstance(cmd, list):
        cmd = "; ".join(cmd)

    return os.system(cmd)


def read_command(cmd: Union[str, List[str]]) -> Tuple[int, str]:
    if isinstance(cmd, list):
        cmd = "; ".join(cmd)

    pipe = os.popen(cmd)
    res = pipe.read()

    return (pipe.close(), res)


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


def get_split_line(splitter: str = '-', text: str = None, length: int = -1) -> str:
    if len(splitter) == 0:
        raise ValueError("Splitter can't be empty.")

    if length <= 0:
        length = get_terminal_size()[1]

    if text is None:
        quotient = int(length / len(splitter))
        remainder = length % len(splitter)

        return splitter * quotient + splitter[:remainder]
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
        splitter_length = len(splitter)
        left = int(remaining / 2)
        if remaining < splitter_length:
            return splitter[:left] + " " + string + " " + splitter[left:remaining]

        right = remaining - left

        quotient_l = int(left / len(splitter))
        remainder_l = left % len(splitter)

        rr = len(splitter) - remainder_l

        quotient_r = int((right - rr) / splitter_length)
        remainder_r = (right - rr) % splitter_length

        return splitter * quotient_l + splitter[:remainder_l] + " " + string + " " + splitter[remainder_l:] + splitter * quotient_r + splitter[:remainder_r]


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
        returncode, res = read_command(
            "git -C {} rev-parse --show-toplevel 2> /dev/null".format(realpath))
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
