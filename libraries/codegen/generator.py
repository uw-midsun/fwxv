from optparse import OptionParser
import jinja2
import os
import yaml


def read_yaml(yaml_file):
    with open(yaml_file, "r") as f:
        data = yaml.load(f, Loader=yaml.FullLoader)
        return data

def get_board_name(yaml_path):
    return yaml_path.split("/")[len(yaml_path.split("/"))-1].split(".")[0]

def get_file_path(template_name, yaml_path, data, dest="./"):
    # get the name of the yaml file (board name) from the filepath
    board = get_board_name(yaml_path)
    data["Board"] = board
    # get the name of the jinja file from the filepath
    jinja_prefix = template_name[:-6]
    # files that start with _ are generic and we want to append the specific name in front
    return dest + (board + jinja_prefix if jinja_prefix[0] == "_" else jinja_prefix)


def write_template(env, template_name, file_path, data):
    template = env.get_template(template_name)
    output = template.render(data=data)
    with open(file_path, "w") as f:
        f.write(output)


def process_setter_data(board, data, master_data):
    for message in data["Messages"]:
        if board in data["Messages"][message].get("target", []) and \
           data["Messages"][message].get("signals", False):
            for signal, length in data["Messages"][message]["signals"].items():
                master_data["Signals"].append((signal, length['length']))

def get_dbc_data():
    yaml_files = get_yaml_files()
    master_data = {"Messages" : []}
    for file in yaml_files:
        data = read_yaml(file)
        sender = get_board_name(file)
        for message_key in data["Messages"]:
            start_bit = 0
            message = data["Messages"][message_key]
            signals = []
            for signal_key in message["signals"]:
                signal = message["signals"][signal_key]
                signals.append({
                    "signal_name": signal_key,
                    "start_bit": start_bit,
                    "length": signal["length"],
                    "scale": signal.get("scale", 1),
                    "offset": signal.get("offset", 0),
                    "min": signal.get("min", 0),
                    "max": signal.get("max", 100),
                    "unit": signal.get("unit", ""),
                    "receiver": " ".join(message["target"])
                })
                start_bit += signal["length"]
            master_data["Messages"].append({
                "id": message["id"],
                "message_name": message_key,
                "signals": signals,
                "data_length": 8,
                "sender": sender
            })
    return master_data

def get_boards_dir():
    # get the working directory to the boards
    working_dir = os.getcwd()
    path_to_file = os.path.dirname(os.path.realpath(__file__))
    return path_to_file.replace("{}/".format(working_dir), "") + "/boards"

def get_yaml_files():
    path_to_boards = get_boards_dir()
    yaml_files = []
    for filename in os.listdir(path_to_boards):
        file_prefix = filename.split(".")[0]  # only get the part before .yaml
        if file_prefix != "boards":
            yaml_files.append(os.path.join(path_to_boards, filename))

    return yaml_files

def get_boards():
    path_to_boards = get_boards_dir()
    for filename in os.listdir(path_to_boards):
        file_prefix = filename.split(".")[0]  # only get the part before .yaml
        if file_prefix == "boards":
            return os.path.join(path_to_boards, filename)

    return False

def parse_board_yaml_files(board):
    yaml_files = get_yaml_files()
    master_data = {"Board": board, "Signals": []}
    for file in yaml_files:
        data = read_yaml(file)
        process_setter_data(board, data, master_data)

    return master_data


def main():
    print("Done autogenerating")


if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-y", "--yaml_file", default=[], dest="yaml_file", action="append",
                      help="yaml file to read", metavar="FILE")
    parser.add_option("-t", "--template", dest="template",
                      help="template file to populate", metavar="FILE")
    parser.add_option("-b", "--board", dest="board", help="which board to generate")

    (options, args) = parser.parse_args()

    templateLoader = jinja2.FileSystemLoader(searchpath="./libraries/codegen/templates")
    env = jinja2.Environment(loader=templateLoader)

    if options.template and "system_can" in options.template:
        data = get_dbc_data()
        boards = get_boards()
        data["Boards"] = read_yaml(boards)["Boards"]
        file_path = "./" + options.template[:-6]
        write_template(env, options.template, file_path, data)
    elif options.board: # only for _setters.h
        data = parse_board_yaml_files(options.board)
        if options.template:
            file_path = "./" + options.board + options.template[:-6]
            write_template(env, options.template, file_path, data)
    else:
        for y in options.yaml_file:
            data = read_yaml(y)
            if options.template:
                file_path = get_file_path(options.template, y, data)
                write_template(env, options.template, file_path, data)

    main()
