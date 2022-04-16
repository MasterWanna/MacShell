import math
import os
import re
from sqlite3 import Connection
from typing import Dict, Iterable, List, Set, Tuple, Union


# utils


line_sep = bytes(os.linesep, encoding="UTF-8")[0]
char_A = b'A'[0]
char_0 = b'0'[0]


def get_terminal_width() -> int:
    return os.get_terminal_size().columns


def get_realpath(path: str) -> str:
    return os.path.realpath(path)


def to_string(strs: Iterable[str], splitter: str = ", ", sort: bool = True) -> str:
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

        string = to_string(textarr, " ", False)

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


# brew


brew_root_db_path = os.getenv("HOME") + "/.config/brew-formula.db"

# brew database


def read_root_nodes(conn: Connection) -> List[str]:
    cursor = conn.execute("select name from 'root-nodes'")
    return [row[0] for row in cursor.fetchall()]


def auto_clean_root_nodes(conn: Connection, root_nodes: Union[List[str], Set[str]], all_nodes: Iterable[str]) -> List[str]:
    remove_list = [node for node in root_nodes if node not in all_nodes]

    if len(remove_list) > 0:
        print("Auto clean index : " + to_string(remove_list))
        conn.executemany("delete from 'root-nodes' where name = ?",
                         [(node,) for node in remove_list])
        conn.commit()

        for node in remove_list:
            root_nodes.remove(node)

    return remove_list


# brew cmdline


def read_all_nodes() -> List[str]:
    pipe = os.popen("brew list --formula")
    lines = pipe.readlines()
    pipe.close()

    return [line.strip() for line in lines]


def read_dependencies() -> Dict[str, List[str]]:
    pipe = os.popen("brew deps --installed --formula")
    lines = pipe.readlines()
    pipe.close()

    items = [line.strip().split(" ") for line in lines]

    return {item[0].split('/')[-1][:-1] if '/' in item[0] else item[0][:-1]: item[1:] for item in items}


def check_formula_available(name: List[str]) -> int:
    return run_command_s("brew info --formula " + to_string(name, " ") + " > /dev/null")


# git


git_repo_db_path = os.getenv("HOME") + "/.config/git-repo.db"

# git database


def read_local_repos(conn: Connection) -> List[str]:
    cursor = conn.execute("select path from 'local-repo' order by path")
    return [row[0] for row in cursor.fetchall()]


# git cmdline


def realpath_repo(repos: List[str]) -> Union[List[str], None]:
    realpath_repos = []

    for path in repos:
        realpath = get_realpath(path)
        returncode, res = read_command("git -C " + realpath +
                                       " rev-parse --show-toplevel 2> /dev/null")
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
        if run_command_s("git -C " + path +
                         " rev-parse --show-toplevel &> /dev/null"):
            git_repo_remove.append(path)
        else:
            git_repo_paths.append(path)

    if len(git_repo_remove) > 0:
        print("Remove non-existent git repo : " + to_string(git_repo_remove))
        conn.executemany("delete from 'local-repo' where path = ?",
                         [(path,) for path in git_repo_remove])
        conn.commit()

    return git_repo_paths
