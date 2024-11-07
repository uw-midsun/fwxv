"""This function reads in yaml files, and populates jinja to make getter and setter functions"""
import argparse
import re
import jinja2
import yaml


def read_yaml(yaml_file):
    """reads yaml file and returns information in data"""
    with open(yaml_file, "r", encoding="utf-8") as f:
        data = yaml.load(f, Loader=yaml.FullLoader)
        return data


def get_function_line_spacing(function_words):
    """Returns an integer that indicates the amount of
    spaces needed leading up to the first open bracket
    Loop through the line array until the first open bracket to find where
    the parameter declaration starts to determine correct spacing"""
    spacing = 0
    bracket_location = -1
    for i, n_m in enumerate(function_words):
        if '(' in n_m:
            bracket_location = i
            break

        spacing += len(n_m)
        if '\n' in n_m:
            spacing -= 1

    if bracket_location == -1:
        raise ValueError("No bracket in provided line!")

    # Count the number of letters leading up to the bracket
    for i in range(len(function_words[bracket_location])):
        spacing += 1
        if function_words[bracket_location][i] == '(':
            break
    return spacing


def format_function_length(function_line, spacing=0):
    """Custom jinja filter that takes in the function string
    and format it to indent to a new line whenever string length
    reaches 100 characters."""

    # Splits by whitespace
    function_line = re.split(r'(\s+)', function_line)
    letter_count = 0

    spacing += get_function_line_spacing(function_line)
    for a_b in enumerate(function_line):
        letter_count += len(a_b)
        if letter_count >= 100:
            a_b = "\n" + (" " * spacing) + a_b
            letter_count = len(a_b)
    function_line = "".join(function_line)
    return function_line


def write_template(env, template_name, file_path, data):
    """writes and creates a file from jinja template and yaml data"""
    template = env.get_template(template_name)
    output = template.render(data=data)
    with open(file_path, "a", encoding="utf-8") as f:
        f.write(output)


def main():
    """prints Test generator"""
    print("Test generator")


if __name__ == "__main__":
    PARSER = argparse.ArgumentParser()
    PARSER.add_argument(
        "-y",
        "--yaml_file",
        dest="yaml_file",
        help="yaml file to read",
        metavar="FILE")
    PARSER.add_argument(
        "-t",
        "--template",
        dest="template",
        default=[],
        action="append",
        help="template file to populate",
        metavar="FILE")
    PARSER.add_argument("-f", "--file_path", default=".", dest="file_path", help="output file path")
    ARGPARSER = PARSER.parse_args()

    DATA = read_yaml(ARGPARSER.yaml_file)

    for template_iterable in ARGPARSER.template:
        template_dir = template_iterable.split('/')[:-1]
        template_name = template_iterable.split('/')[-1]
        templateLoader = jinja2.FileSystemLoader(searchpath=template_dir)
        env = jinja2.Environment(loader=templateLoader)
        env.filters["format_function_length"] = format_function_length
        file_path = ARGPARSER.file_path + "/" + template_name[:-6] + ".c"

        write_template(env, template_name, file_path, DATA)

    main()
