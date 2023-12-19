import threading
import subprocess
import re


def set_output(output):
    '''set the terminal output to the output string'''
    # 'static' variable
    if "output" not in set_output.__dict__:
        set_output.output = ""
    if output != set_output.output:
        print("\033c", end="")
        print(output)
        set_output.output = output


CANDUMP_PATTERN = re.compile(r"(\w*)  (\w*)   \[([0-9])\]  ((?:\w\w ?)+)")


def parse_line(line):
    '''get the can id and data from a candump line'''
    _, can_id, length, data = CANDUMP_PATTERN.match(line).groups()
    can_id = int(can_id, base=16)
    length = int(length)
    data = bytes.fromhex(data)

    return can_id, data


class Can:
    def __init__(self, device) -> None:
        '''create a can controller, can be used to send and receive can messages to a can line'''
        self.device = device
        self.rx = {}
        self.rx_thread = threading.Thread(target=self.rx_all)
        self.rx_thread.start()

    def rx_all(self):
        proc = subprocess.Popen(["candump", self.device])
        while True:
            line = proc.stdout.readline().decode()
            can_id, data = parse_line(line)

            # do things

    def tx(self, message):
        subprocess.run(["cansend", self.device, message], check=False)
