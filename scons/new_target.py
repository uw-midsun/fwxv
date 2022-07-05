#!/usr/bin/env python3
"""New target script.

This module sets up the folder structure and config.json for a new project or library.

Usage: python3 new_target.py project|library name
"""
import os
import argparse
import textwrap
import json
from string import Template

README_TEMPLATE = Template("""\
    <!--
        General guidelines
        These are just guidelines, not strict rules - document however seems best.
        A README for a firmware-only project (e.g. Babydriver, MPXE, bootloader, CAN explorer) should answer the following questions:
            - What is it?
            - What problem does it solve?
            - How do I use it? (with usage examples / example commands, etc)
            - How does it work? (architectural overview)
        A README for a board project (powering a hardware board, e.g. power distribution, centre console, charger, BMS carrier) should answer the following questions:
            - What is the purpose of the board?
            - What are all the things that the firmware needs to do?
            - How does it fit into the overall system?
            - How does it work? (architectural overview, e.g. what each module's purpose is or how data flows through the firmware)
    -->
    # $name

    """)


DEFAULT_DEPS = ["FreeRTOS", "ms-common"]

main_template_file = open(os.path.join('scons', 'template', 'main_template.txt'), 'r');

def generate_config(target_type):
    """Generates a new config.json file for a project/library.

    Args:
        target_type: Either 'project' or 'library'

    Returns:
        dict: dictionary representation of the initial config.json file.
    """
    deps = DEFAULT_DEPS if target_type == 'project' or target_type == 'smoke' else []

    return {"libs": deps}



def new_target(target_type, name):
    """Creates a new project or library.

    Creates a subfolder in the appropriate folder with the specified name
    with the following structure:

    projects/libraries
    - name
        - inc
        - README.md
        - config.json 
        - src
        - test

    where config.json is required for the project or library to be valid.

    Args:
        target_type: Either 'project' or 'library'.
        name: The new project or library's name.

    Returns:
        None
    """
    type_folders = {
        'project': 'projects',
        'library': 'libraries',
        'smoke': 'smoke'
    }

    proj_path = os.path.join(type_folders[target_type], name)
    folders = ['src', 'inc', 'test']

    for folder in folders:
        os.makedirs(os.path.join(proj_path, folder), exist_ok=True)

    with open(os.path.join(proj_path, 'config.json'), 'w') as config_file:
        json.dump(generate_config(target_type), config_file, indent=4)

    with open(os.path.join(proj_path, 'README.md'), 'w') as readme_file:
        readme_file.write(textwrap.dedent(README_TEMPLATE.substitute({'name': name})))

    with open(os.path.join(proj_path, 'src', 'main.c'), 'w') as main_file:
        main_file.write(textwrap.dedent(main_template_file.read()))

    print('Created new {0} {1}'.format(target_type, name))


def main():
    """Main entry point of program"""
    parser = argparse.ArgumentParser(description='Creates new project/library')
    parser.add_argument('type', choices=['project', 'library', 'smoke'])
    parser.add_argument('name')
    args = parser.parse_args()

    new_target(args.type, args.name)


if __name__ == '__main__':
    main()
