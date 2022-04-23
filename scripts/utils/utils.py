from __future__ import annotations
import math
import os
import re
from sqlite3 import Connection
from typing import Any, Dict, Iterable, List, Set, Tuple, Type, Union

# computation utils


char_A = b'A'[0]
char_0 = b'0'[0]


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


class reverse(object):
    def __init__(self, obj: Any):
        self.obj = obj

    def __str__(self) -> str:
        return "reverse({})".format(self.obj)

    def __repr__(self) -> str:
        return "reverse({})".format(self.obj)

    def __lt__(self, other: reverse) -> bool:
        return not self.obj < other.obj

    def __le__(self, other: reverse) -> bool:
        return not self.obj <= other.obj

    def __eq__(self, other: reverse) -> bool:
        return self.obj == other.obj


class boolean:
    TRUE = ["true", "t", "1", "yes", "y"]

    def __init__(self, value: Any):
        if isinstance(value, str):
            self.value = value.lower() in self.TRUE
        else:
            self.value = bool(value)

    def __bool__(self) -> bool:
        return bool(self.value)

    def __repr__(self) -> str:
        return str(self.value)

    def __str__(self) -> str:
        return str(self.value)


class convent:
    def __init__(self, key_sep: str = ',', item_sep: str = ';', item: List[Tuple[bool, Type, Any]] = None, items: int = None) -> None:
        if item is None and items is None:
            raise ValueError('item or items must be specified')
        if key_sep is None:
            raise ValueError('key_sep must be specified')
        if item_sep is None:
            raise ValueError('item_sep must be specified')

        self.key_sep = key_sep
        self.item_sep = item_sep
        if item is not None:
            self.items = len(item)
            self.item = item
        else:
            self.items = items
            self.item = [(False, str, None)] * items

        assert len(self.item) == self.items, "item must be a list of length items"

        for index in range(self.items):
            assert len(self.item[index]) == 3, \
                "item must be a list of length 3"

        for index in range(self.items - 1):
            assert self.item[index][0] or not self.item[index + 1][0], \
                "required item must be before optional item"

        self.required = sum([x for x, _, _ in self.item])

    def convert(self, arguments: List[str]) -> List[List[Any]]:
        elements = []
        for element in arguments:
            elements.extend(element.split(self.item_sep))

        all_items: List[str] = [arg.split(self.key_sep) for arg in elements]

        result = []

        for item in all_items:
            assert self.required <= len(item) <= self.items, \
                "item must be a list of length less than items"

            item_result = []
            for index in range(len(item)):
                item_result.append(self.item[index][1](item[index]))

            for index in range(len(item), self.items):
                item_result.append(self.item[index][1](self.item[index][2]))

            result.append(item_result)

        return result

    def __call__(self, arguments: List[str]) -> List[List[Any]]:
        return self.convert(arguments)


# command line and terminal utils
line_sep = bytes(os.linesep, encoding="UTF-8")[0]


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


# brew


brew_root_db_path = os.getenv("HOME") + "/.config/brew-formula.db"
brew_root_table_name = "root-node"


# brew database


def read_root_nodes(conn: Connection) -> List[str]:
    cursor = conn.execute("select name from '{}'".format(brew_root_table_name))
    return [row[0] for row in cursor.fetchall()]


def auto_clean_root_nodes(conn: Connection, root_nodes: Union[List[str], Set[str]], all_nodes: Iterable[str]) -> List[str]:
    remove_list = [node for node in root_nodes if node not in all_nodes]

    if len(remove_list) > 0:
        print("Auto clean index : " + to_string(remove_list, sort=True))
        conn.executemany("delete from '{}' where name = ?".format(brew_root_table_name),
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
    return run_command_s("brew info --formula " + to_string(name, " ", True) + " > /dev/null")


# git


git_repo_db_path = os.getenv("HOME") + "/.config/git-repo.db"
git_repo_table_name = "local-repo"


# git database


def read_local_repos(conn: Connection) -> List[str]:
    cursor = conn.execute("select path from '{}' order by path".format(git_repo_table_name))
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
        print("Remove non-existent git repo : " +
              to_string(git_repo_remove, sort=True))
        conn.executemany("delete from '{}' where path = ?".format(git_repo_table_name),
                         [(path,) for path in git_repo_remove])
        conn.commit()

    return git_repo_paths
