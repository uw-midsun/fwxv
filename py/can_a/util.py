import re

CANDUMP_PATTERN = re.compile(r"(\w*)  (\w*)   \[([0-9])\]  ((?:\w\w ?)+)")


def set_output(output):
    '''set the terminal output to the output string'''
    # 'static' variable
    if "output" not in set_output.__dict__:
        set_output.output = ""
    # print("OUTPUTS SET")
    if output != set_output.output:
        print("\033c", end="")
        print(output)
        set_output.output = output


def parse_line(line):
    '''get the can id and data from a candump line'''
    _, can_id, length, data = CANDUMP_PATTERN.match(line).groups()
    # if (match == N)
    can_id = int(can_id, base=16)
    length = int(length)
    data = bytes.fromhex(data)

    return can_id, data
