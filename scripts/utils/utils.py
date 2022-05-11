import math
import os
from pathlib import Path
import re
from sqlite3 import Connection
from typing import Iterable, List, Tuple, Union

# computation utils


char_A = b'A'[0]
char_0 = b'0'[0]


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
linesep = os.linesep
line_sep = bytes(linesep, encoding="UTF-8")[0]


def get_filename(path: str) -> str:
    return Path(path).stem


def get_file_fullname(path: str) -> str:
    return path.split(os.sep)[-1]


def get_terminal_width() -> int:
    try:
        return os.get_terminal_size().columns
    except OSError:
        return 0


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


def run_command_s(cmd: str) -> int:
    return run_command(cmd, False)


def run_command(cmd: str, echo: bool = True) -> int:
    if echo:
        print(cmd)

    return os.system(cmd)


def read_command(cmd: str) -> Tuple[int, str]:
    pipe = os.popen(cmd)
    res = pipe.read()

    return (pipe.close(), res)


def get_split_line(splitter: str = '-', text: str = None, length: int = get_terminal_width()) -> str:
    if len(splitter) == 0:
        raise ValueError("Splitter can't be empty.")

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


git_repo_db_path = "{}/.config/git-repo.db".format(HOME)
git_repo_table_name = "local-repo"


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
