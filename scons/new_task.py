import os
import sys
import jinja2
import yaml
import argparse
import re

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
        raise Exception("No bracket in provided line!")

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

def new_task(type, name, task_name):
    folder_choice = {
        'project' : 'projects',
        'library' : 'libraries'
    }
    subfolders = ['src', 'inc']

    folder_path = os.path.join(folder_choice[type], name)
    
    for folder in subfolders:
        os.makedirs(os.path.join(folder_path, folder), exist_ok=True)

    DATA = {
        "task_name" : task_name,
        "proj_name" : name
    }
    src_path = os.path.join(folder_path, subfolders[0])
    inc_path = os.path.join(folder_path, subfolders[1])

    for template_iterable in ["_task.c.jinja", "_task.h.jinja"]:
        template_name = template_iterable
        templateLoader = jinja2.FileSystemLoader(searchpath="scons/template")
        env = jinja2.Environment(loader=templateLoader)
        env.filters["format_function_length"] = format_function_length

        if ".h.jinja" in template_name:
            file_path = inc_path + "/" + name + "_" + task_name + template_name[:-6]
        else:
            file_path = src_path + "/" + name + "_" + task_name + template_name[:-6]

        write_template(env, template_name, file_path, DATA)

def make_new_task(target, source, env):
    TYPE = env["VARS"]["TYPE"]
    TARGET = env["VARS"]["TARGET"]
    NAME = env["VARS"]["NAME"]
    # No project or library option provided
    if TYPE not in ['project', 'libray']:
        print("Missing project or library name. Expected --project=..., or --library=...")
        sys.exit(1)

    # Chain or's to select the first non-None value 
    new_task(TYPE, TARGET, NAME)

def main():
    PARSER = argparse.ArgumentParser()
    PARSER.add_argument("-c",'--type', choices=['project', 'library'], dest="type")
    PARSER.add_argument("--task_name", dest="task_name")
    PARSER.add_argument("-n",'--name', dest="name")
    ARGPARSER = PARSER.parse_args()
    new_task(ARGPARSER.type, ARGPARSER.name, ARGPARSER.task_name)

if __name__ == '__main__':
    main()
