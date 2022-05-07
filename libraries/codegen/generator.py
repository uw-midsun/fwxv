from optparse import OptionParser
import jinja2
import os
import yaml
import re


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

def get_template_spacing(template):
    # Returns an integer that indicates the amount of spaces needed leading up to the first open bracket
    # Loop through the template until the first open bracket to find where the parameter declaration starts to determine correct spacing
    spacing = 0
    start_of_parameter = 0
    for i in range(len(template)):
        if '(' in template[i]:
            start_of_parameter = i
            break
        else:
            if '\n' in template[i]:
                spacing += len(template[i]) - 1
            else:
                spacing += len(template[i])

    # Count the number of letters leading up to the bracket
    for i in range(len(template[start_of_parameter])):
        if template[start_of_parameter][i] == '(':
            spacing += 1
            break
        else:
            spacing += 1
    return spacing

def format_function_parameters(template):
    # Custom jinja filter that takes in the function parameter string
    # and format it to indent to a new line whenever string length 
    # reaches 100 characters.

    template = re.split(r'(\s+)', template)
    letter_count = 0

    parameter_tabbing_space = get_template_spacing(template)
    for i in range(len(template)):
        letter_count += len(template[i])
        if letter_count >= 100:
            template[i] = "\n" + (" "*parameter_tabbing_space) + template[i]
            letter_count = len(template[i])
    template = "".join(template)
    return (template)

def format_function_body(template):
    # Custom jinja filter that takes in the function body string
    # and format it to indent to a new line whenever string length 
    # reaches 100 characters.
    
    template = re.split(r'(\s+)', template)
    template[0] = '  ' + template[0]
    letter_count = 0
    
    function_body_spacing = get_template_spacing(template=template)
    for i in range(len(template)):
        letter_count += len(template[i])
        if letter_count >= 100:
            template[i] = "\n" + (" "*function_body_spacing) + template[i]
            letter_count = len(template[i])
    template = "".join(template)
    return (template)

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
    master_data = {"Messages" : [], "Boards": get_boards()}
    for can_board_id, board in enumerate(master_data["Boards"]):
        data = read_yaml(yaml_files[board])
        sender = get_board_name(yaml_files[board])
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
                "id": (message["id"] << 5) + can_board_id,
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
    yaml_files = {}
    for file in os.listdir(path_to_boards):
        board_name = file.split(".")[0]  # only get the part before .yaml
        if board_name != "boards":
            yaml_files[board_name] = os.path.join(path_to_boards, file)

    return yaml_files

def get_boards():
    path_to_boards = get_boards_dir()
    for file in os.listdir(path_to_boards):
        board_name = file.split(".")[0]  # only get the part before .yaml
        if board_name == "boards":
            boards_yaml = os.path.join(path_to_boards, file)
            return read_yaml(boards_yaml)["Boards"]

    return []

def parse_board_yaml_files(board):
    master_data = {"Board": board, "Signals": []}
    yaml_files = list(get_yaml_files().values())
    for file in yaml_files:  
        data = read_yaml(file)
        process_setter_data(board, data, master_data)

    return master_data

def check_yaml_file(data):
    regex = re.compile('[@!#$%^&*()<>?/\|}{~:]')
    message_ids = set()

    for message in data["Messages"]:
        # Message has id
        if "id" not in data["Messages"][message]:
            raise Exception("Message " + message + " has no id")
        # No same ids for messages within a message
        if data["Messages"][message]["id"] in message_ids:
            raise Exception("Duplicate message id")
        # All ids are between 0-64
        elif data["Messages"][message]["id"] > 64 or data["Messages"][message]["id"] < 0:
            raise Exception("Invalid message id")
        else:
            message_ids.add(data["Messages"][message]["id"])
        # No illegal characters in message names
        if(regex.search(message) != None):
            raise Exception("Illegal character in message name")
        # Doesn't have more than 8 signals per message
        if len(data["Messages"][message]["signals"]) > 8:
            raise Exception("More than 8 signals in a message")

        signal_len = data["Messages"][message]["signals"][next(iter(data["Messages"][message]["signals"]))]["length"]
        for signal in data["Messages"][message]["signals"]:
            # All signals within a message are the same length
            if data["Messages"][message]["signals"][signal]["length"] != signal_len:
                raise Exception("Signal length mismatch")
            # No illegal characters in signal names
            if(regex.search(signal) != None):
                raise Exception("Illegal character in signal name")

def main():
    print("Done autogenerating")


if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-y", "--yaml_file", default=[], dest="yaml_file", action="append",
                      help="yaml file to read", metavar="FILE")
    parser.add_option("-t", "--template", dest="template",
                      help="template file to populate", metavar="FILE")
    parser.add_option("-b", "--board", default=None, dest="board", help="which board to generate")
    parser.add_option("-f", "--file_path", default=".", dest="file_path", help="output file path")

    (options, args) = parser.parse_args()

    if options.template:
        codegen_dir = '/'.join(options.template.split('/')[:-1])
        template_name = options.template.split('/')[-1]
        templateLoader = jinja2.FileSystemLoader(searchpath=codegen_dir)
        env = jinja2.Environment(loader=templateLoader)
        env.filters["format_function_parameters"] = format_function_parameters
        env.filters["format_function_body"] = format_function_body
        file_path = options.file_path + "/" + get_file_name(template_name, options.board)

        if "system_can.dbc" in template_name:
            data = get_dbc_data()
            if len(data["Boards"]):
                write_template(env, template_name, file_path, data)
        elif options.yaml_file:
            for y in options.yaml_file:
                data = read_yaml(y)
                if options.board:
                    data["Board"] = options.board
                if data.get("Boards"):
                    data["Boards"] = [
                        parse_board_yaml_files(data["Boards"][i]) for i in range(len(data["Boards"]))
                    ]
                write_template(env, template_name, file_path, data)
        elif options.board:
            data = parse_board_yaml_files(options.board)
            write_template(env, template_name, file_path, data)
        

    main()
