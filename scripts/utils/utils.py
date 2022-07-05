from argparse import ArgumentParser
import fcntl
import math
import multiprocessing
import os
from pathlib import Path
import pickle
import re
import signal
import sys
import tty
import urllib.request as urllib
from sqlite3 import Connection
import struct
import termios
from typing import Any, Callable, Dict, Iterable, List, Tuple, Union

# computation utils


char_A = ord('A')
char_0 = ord('0')
charlen_obj = None
charlen_key = None


def get_value_ignore_case(dic: Dict[str, Any], key: str, default: Any = None) -> Any:
    for k, v in dic.items():
        if k.lower() == key.lower():
            return v
    return default


def exec_all(func: Callable, args: List[Any], parallel: bool = True) -> List[Any]:
    if parallel:
        pool = multiprocessing.Pool(processes=multiprocessing.cpu_count())

        res = pool.map(func, args)

        pool.close()
        pool.join()

    else:
        res = [func(arg) for arg in args]

    return res


def get_char_real_len(string: str) -> int:
    global charlen_obj, charlen_key
    if charlen_obj is None:
        if not os.path.exists(charlen):
            print("Must run `charlen` first!")
            run_command("charlen")
        charlen_obj = pickle.load(open(charlen, "rb"))
        charlen_key = sorted(charlen_obj.keys(), reverse=True)

    string = string[0]

    for k in charlen_key:
        if k <= string:
            return charlen_obj[k]

    raise Exception("Invalid character: {}".format(string))


def get_real_string_len(string: str) -> int:
    return int(sum(get_char_real_len(ch) for ch in string))


def get_transformation_len(num: int, base: int) -> int:
    return math.ceil(math.log(num, base))


def byte_max_len(base: int) -> int:
    return get_transformation_len(0xff, base)


def char_max_len(base: int) -> int:
    return get_transformation_len(0x10ffff, base)


def format_int_base(num: int, base: int, length = 0) -> str:
    format_num = ""

    while num != 0:
        quotient = int(num / base)
        remainder = num % base

        if remainder > 9:
            format_num = chr(char_A + remainder - 10) + format_num
        else:
            format_num = chr(char_0 + remainder) + format_num

        num = quotient

    if length > 0:
        format_num = format_num.zfill(length)

    return format_num


# command line and terminal utils


HOME = os.getenv("HOME")
db_path = "{}/.config/shell-db.db".format(HOME)
charlen = "{}/.config/term-charlen.pkl".format(HOME)
VM_NAME_MAPPING = {
    "redhat": "Red Hat"
}

linesep = os.linesep
line_sep = ord(linesep)


def getchar() -> str:
    input = open("/dev/tty", "r")
    fd = input.fileno()
    old_mode = termios.tcgetattr(fd)
    try:
        mode = termios.tcgetattr(fd)
        mode[tty.LFLAG] = mode[tty.LFLAG] & ~termios.ICANON & ~termios.ECHO
        termios.tcsetattr(fd, termios.TCSANOW, mode)
        ch = input.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSANOW, old_mode)
    return ch


def run_with_timeout(func: Callable, args: List[Any], timeout: int) -> Any:
    def handler(signum, frame):
        raise Exception("Timeout")

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(timeout)

    try:
        res = func(*args)
    except:
        return None

    signal.alarm(0)

    return res


def run_command(cmd: Union[str, List[str]], input: Union[Any, List[Any]] = None) -> int:
    if not isinstance(cmd, str):
        cmd = "; ".join(cmd)

    if input:
        pipe = os.popen(cmd, "w")
        if not isinstance(input, str):
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


def exec_command(cmd: str, text: str = None, line: bool = False, end: bool = True) -> str:
    if isinstance(cmd, str):
        command = cmd
    else:
        command = to_string(cmd, ' ')

    text = text if text else command
    
    if line:
        print(get_split_line(text=text))
    else:
        print(get_split_block(text=text))
    
    run_command(command)

    if end and not line:
        print(get_split_block(text="Done!"))


def read_file(path: str) -> str:
    with open(path, "r") as f:
        return f.read()


def read_webpage(url: str) -> str:
    try:
        with urllib.urlopen(url) as f:
            return f.read().decode("utf-8")
    except:
        return ""


def write_file(path: str, content: Union[str, List[str]], sep: str = linesep) -> None:
    if isinstance(content, str):
        content = [content]

    with open(path, "w") as f:
        for i, line in enumerate(content):
            if i > 0:
                f.write(sep)
            f.write(line)

        f.write(linesep)

def get_filename(path: str) -> str:
    return Path(path).stem


def extend_filename(path: str, extension: str) -> str:
    finfo = Path(path)
    return finfo.stem + extension + finfo.suffix


def get_file_fullname(path: str) -> str:
    return path.split(os.sep)[-1]


def get_terminal_size():
    """
        import curses
        win = curses.initscr()
        size = win.getmaxyx()
        curses.endwin()
        print(size)
    """
    # above code have extra bytes before printed out string but can also get correct terminal size

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
    max_len = max(get_real_string_len(s) for s in items)

    len_diff = (get_real_string_len(s) - len(s) for s in items)

    if width <= 0:
        width = get_terminal_size()[1]

    items_line = (width + 1) // (max_len + 1)

    formatted = ""

    for i in range(len(items)):
        if (i + 1) % items_line == 0:
            formatted += items[i] + linesep
        else:
            formatted += items[i].ljust(max_len - next(len_diff) + 1)

    return formatted


def get_realpath(path: str) -> str:
    return os.path.realpath(path)


def to_string(strs: Iterable[str], splitter: str = ", ", sort: bool = False) -> str:
    if sort:
        return splitter.join(sorted(strs))
    else:
        return splitter.join(strs)


def get_split_line(splitter: str = '-', text: str = None, length: int = 0) -> str:
    if len(splitter) != 1:
        raise ValueError("Splitter should a single char.")

    if length <= 0:
        length = get_terminal_size()[1] + length

    if length <= 0:
        raise ValueError("Length value out of range.")

    real_len = get_real_string_len(splitter)

    if text is None:
        return "\r" + splitter * (length // real_len)
    else:
        textarr = re.split(r'\s+', text)

        if len(textarr) > 0 and len(textarr[0]) == 0:
            textarr.pop(0)
        if len(textarr) > 0 and len(textarr[-1]) == 0:
            textarr.pop(-1)

        if len(textarr) == 0:
            return get_split_line(splitter, None)

        string = to_string(textarr, " ")

        str_length = get_real_string_len(string)
        if str_length + 4 > length:
            raise ValueError("Text too long : " + string)

        remaining = (length - str_length - 2) // real_len
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

    real_len = get_real_string_len(splitter)

    block = get_split_line(splitter, length=length) + linesep

    for line in text:
        block += splitter * width + get_split_line(" ", line, (length // real_len - width * 2) * real_len)[1:] + splitter * width + linesep

    block += get_split_line(splitter, length=length)

    return "\r" + block


def list_dict():
    pass


def list_table(title: Union[List[str], int], args: List[List[Any]], format: List[str] = None) -> str:
    titled = not isinstance(title, int)
    if titled:
        cols = len(title)
    else:
        cols = title

    for arg in args:
        if len(arg) != cols:
            raise ValueError("Argument length mismatch.")

    if format:
        if len(format) != cols:
            raise ValueError("Format length mismatch.")
    else:
        format = ["{}"] * cols
    
    str_args = [[format[i].format(arg[i]) for i in range(cols)] for arg in args]

    if titled:
        str_args.insert(0, title)

    max_len = [max([get_real_string_len(str_arg[i]) for str_arg in str_args]) for i in range(cols)]

    table = get_split_line() + linesep
    
    for str_arg in str_args:
        arg_str = to_string((str_arg[i].ljust(max_len[i] - get_real_string_len(str_arg[i]) + len(str_arg[i])) for i in range(cols)), "  ") + linesep

        table += arg_str + get_split_line() + linesep
        
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
        if run_command("git -C {} rev-parse --show-toplevel &> /dev/null".format(path)):
            git_repo_remove.append(path)
        else:
            git_repo_paths.append(path)

    if len(git_repo_remove) > 0:
        print("Remove non-existent git repo : " +
              to_string(git_repo_remove, sort=True))
        conn.executemany("delete from '{}' where path = ?".format(git_repo_table_name),
                         ((path,) for path in git_repo_remove))
        conn.commit()

    return git_repo_paths


if __name__ == "__main__":
    parser = ArgumentParser(description="MacShell Utils")

    parser.parse_args()

    print("{} provides varaible and functions for MacShell.".format(get_file_fullname(__file__)))
