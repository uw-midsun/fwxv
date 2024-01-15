import argparse
import jinja2
import yaml
import re
from pathlib import Path


def get_file_name(template_name, board):
    # get the name of the jinja file from the filepath
    jinja_prefix = Path(template_name).stem
    # files that start with _ are generic and we want to prepend the board name
    if jinja_prefix.startswith('_') and board != None:
        return board + jinja_prefix
    else:
        return jinja_prefix


def check_yaml_file(data):
    illegal_chars_regex = re.compile('[@!#$%^&*()<>?/\|}{~:]')
    message_ids = set()

    for message_name, message in data["Messages"].items():
        # Message has id
        if "id" not in message:
            raise Exception("Message " + message_name + " has no id")

        # No same ids for messages within a board
        if message["id"] in message_ids:
            raise Exception("Duplicate id for message" + message_name)
        # All ids are between 0-64
        elif message["id"] >= 64 or message["id"] < 0:
            raise Exception("Invalid message id")
        else:
            message_ids.add(message["id"])

        # No illegal characters in message names
        if (illegal_chars_regex.search(message_name) != None):
            raise Exception("Illegal character in message name")

        # Doesn't have more than 8 signals per message
        if len(message["signals"]) > 8:
            raise Exception("More than 8 signals in a message")

        message_length = 0
        for signal_name, signal in message["signals"].items():
            # No illegal characters in signal names
            if (illegal_chars_regex.search(signal_name) != None):
                raise Exception("Illegal character in signal name")
            # All signals within a message are the same length
            if signal["length"] % 8 != 0:
                raise Exception("Signal length must be a multiple of 8")
            message_length += signal['length']

        if message_length > 64:
            raise Exception("Message must be 64 bits or less")


def get_data():
    boards = []
    messages = []

    for yaml_path in Path(__file__).parent.glob("boards/*.yaml"):
        # read yaml
        with open(yaml_path, "r") as f:
            data = yaml.load(f, Loader=yaml.FullLoader)
            check_yaml_file(data)  # check data is valid

        sender = Path(yaml_path).stem
        boards.append(sender)

        for message_name, message in data["Messages"].items():
            signals = []
            start_bit = 0
            for signal_name, signal in message["signals"].items():
                signals.append({
                    "name": signal_name,
                    "start_bit": start_bit,
                    "length": signal["length"],
                    "scale": 1,
                    "offset": 0,
                    "min": 0,
                    "max": 100,
                    "receiver": message["target"],
                })
                start_bit += signal["length"]

            messages.append({
                "id": message["id"],
                "name": message_name,
                "signals": signals,
                "sender": sender,
                "receiver": message["target"],
            })

    return {"Boards": boards, "Messages": messages}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--template", nargs='+', default=[], dest="templates",
                        action="append", help="template file to populate", metavar="FILE")
    parser.add_argument("-f", "--file_path", default=[], dest="outputs",
                        action="append", help="output directory path", metavar="DIR")
    parser.add_argument("-b", dest="board", default=None)

    args = parser.parse_args()
    data = get_data()
    data.update({"Board": args.board})

    template_loader = jinja2.FileSystemLoader(
        searchpath=Path(__file__).parent.joinpath("templates").as_posix())
    env = jinja2.Environment(loader=template_loader)
    env.tests["contains"] = (lambda list, var: (var in list))

    for output_dir, templates in zip(args.outputs, args.templates):
        for template in templates:
            output = env.get_template(template).render(data=data)
            Path(output_dir, get_file_name(template, args.board)).write_text(output)

    print("Done autogenerating")


if __name__ == "__main__":
    main()
