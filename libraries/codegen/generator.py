import argparse
import jinja2
import os
import yaml
import re

# jinja2 custom test for if a list contains a variable 
def contains(list, var):
    return (var in list)

def file_no_ext(path):
    # return file name without extension
    return os.path.splitext(os.path.basename(path))[0]

def get_file_name(template_name, board):
    # get the name of the jinja file from the filepath
    jinja_prefix = file_no_ext(template_name)
    # files that start with _ are generic and we want to prepend the board name
    if jinja_prefix.startswith('_') and board != None:
        return board + jinja_prefix
    else:
        return jinja_prefix

def read_yaml(yaml_file):
    with open(yaml_file, "r") as f:
        data = yaml.load(f, Loader=yaml.FullLoader)
        # Only check if a board is specified
        if "Boards" in data.keys():
            return data
        elif "Messages" in data.keys():
            check_yaml_file(data)
        else:
            raise Exception("No message or board present in yaml")
        return data

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
        if(illegal_chars_regex.search(message_name) != None):
            raise Exception("Illegal character in message name")
        
        # Doesn't have more than 8 signals per message
        if len(message["signals"]) > 8:
            raise Exception("More than 8 signals in a message")

        message_length = 0
        for signal_name, signal in message["signals"].items():
            # No illegal characters in signal names
            if(illegal_chars_regex.search(signal_name) != None):
                raise Exception("Illegal character in signal name")
            # All signals within a message are the same length
            if signal["length"] % 8 != 0:
                raise Exception("Signal length must be a multiple of 8")
            message_length += signal['length']
            
        if message_length > 64:
            raise Exception("Message must be 64 bits or less")

def get_data():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    boards_dir = os.path.join(current_dir, "boards")

    boards = []
    messages = []

    for yaml_path in os.listdir(boards_dir):
        data = read_yaml(os.path.join(boards_dir, yaml_path))
        sender = file_no_ext(yaml_path)
        boards.append(sender)

        for message_name, message in data["Messages"].items():
            signals = []
            start_bit = 0
            for signal_name, signal in message["signals"].items():
                signals.append({
                    "name": signal_name,
                    "start_bit": start_bit,
                    "length": signal["length"],
                })
                start_bit += signal["length"]
            
            messages.append({
                "id": message["id"],
                "name": message_name,
                "signals": signals,
                "sender": sender,
                "receiver": message["target"],
            })

    return { "Boards": boards, "Messages": messages }

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--template", nargs='+', default=[], dest="templates", action="append",
                      help="template file to populate", metavar="FILE")
    parser.add_argument("-f", "--file_path", default=[], dest="file_path", action="append", help="output file path")
    parser.add_argument("-b", dest="board", default=None)

    args = parser.parse_args()
    data = get_data()
    data.update({ "Board": args.board })

    current_dir = os.path.dirname(os.path.abspath(__file__))
    template_dir = os.path.join(current_dir, "templates")

    template_loader = jinja2.FileSystemLoader(searchpath=template_dir)
    env = jinja2.Environment(loader=template_loader)
    env.tests["contains"] = contains

    for file_path, templates in zip(args.file_path, args.templates):
        for template in templates:
            output = env.get_template(template).render(data=data)
            with open(os.path.join(file_path, get_file_name(template, args.board)), "w") as f:
                f.write(output)

    print("Done autogenerating")

if __name__ == "__main__":
    main()
