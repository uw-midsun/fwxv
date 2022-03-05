from optparse import OptionParser
import jinja2
import os
import yaml


def read_yaml(yaml_file):
    with open(yaml_file, "r") as f:
        data = yaml.load(f, Loader=yaml.FullLoader)
        return data

def format_pack_h(env, template_name, file_path, data):
    # get the template as a string and split data into list
    template = env.get_template(template_name)
    raw_data = template.render(data=data).split()
    letter_count = 0
    
    # traverse through the list to format the code to a correct header file format
    for i in range(len(raw_data)):
        letter_count += len(raw_data[i])
        if letter_count >= 100:
            letter_count = 0
            raw_data[i] = ("\n"+(" "*(spacing_legnth + 1)) + raw_data[i])
            letter_count += len(raw_data[i])

        # checking for include statements
        if raw_data[i].startswith("#"):
            if raw_data[i] == "#pragma":
                raw_data[i + 1] += "\n\n"
                letter_count = 0
            elif raw_data[i] == "#include":
                raw_data[i + 1] += "\n"
                letter_count = 0

        # checking for the start of a function definition
        if raw_data[i] == "void" and raw_data[i + 1].startswith("can_pack"):
            letter_count = 1
            spacing_legnth = len(raw_data[i] + raw_data[i + 1]) - 9
            raw_data[i] = ("\n" + raw_data[i])
            letter_count += len(raw_data[i])

        # checking for when spacing would be needed/independent words to account for the space
        elif raw_data[i].endswith(",") or raw_data[i].endswith("_t") :
            letter_count += 1

        # check for when going to the function body
        if raw_data[i].endswith("{"):
            spacing_legnth = len(raw_data[i + 1]) - 8
            raw_data[i] += ("\n"+" "*2)
            letter_count = 2

        # check for when function body ends
        elif raw_data[i].endswith("}"):
            raw_data[i] = ("\n" + raw_data[i] + "\n")
            letter_count = 0
    
    raw_data = " ".join(raw_data).replace("\n ", "\n")
    # writes data to .h template
    with open(file_path, "w") as f:
       f.write(raw_data)
    

def get_board_name(yaml_path):
    return yaml_path.split("/")[len(yaml_path.split("/"))-1].split(".")[0]

def get_file_name(template_name, board):
    # get the name of the jinja file from the filepath
    jinja_prefix = template_name[:-6]
    # files that start with _ are generic and we want to prepend the board name
    return board + jinja_prefix if jinja_prefix[0] == "_" and board else jinja_prefix


def write_template(env, template_name, file_path, data):
    template = env.get_template(template_name)
    output = template.render(data=data)
    with open(file_path, "w") as f:
        f.write(output)


def process_setter_data(board, data, master_data):
    for message in data["Messages"]:
        if board in data["Messages"][message].get("target", []) and \
           data["Messages"][message].get("signals", False):
            for signal, signal_data in data["Messages"][message]["signals"].items():
                signal_data["message"] = message
                master_data["Signals"].append((signal, signal_data))

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
    parser.add_option("-b", "--board", default=None, dest="board", help="which board to generate")
    parser.add_option("-f", "--file_path", dest="file_path", help="output file path")

    (options, args) = parser.parse_args()

    if options.template:
        codegen_dir = '/'.join(options.template.split('/')[:-1])
        template_name = options.template.split('/')[-1]
        templateLoader = jinja2.FileSystemLoader(searchpath=codegen_dir)
        env = jinja2.Environment(loader=templateLoader)
        file_path = options.file_path + "/" + get_file_name(template_name, options.board)

        if "system_can" in template_name:
            data = get_dbc_data()
            boards = get_boards()
            data["Boards"] = read_yaml(boards)["Boards"]
            write_template(env, template_name, file_path, data)
        elif options.board and options.yaml_file:
            for y in options.yaml_file:
                data = read_yaml(y)
                if "pack" in template_name:
                    format_pack_h(env, template_name, file_path, data)
                else:
                    write_template(env, template_name, file_path, data)
        elif options.board:
            data = parse_board_yaml_files(options.board)
            write_template(env, template_name, file_path, data)
        

    main()
