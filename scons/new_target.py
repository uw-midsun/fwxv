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


MAIN_TEMPLATE = """\
    #include <stdio.h>

    #include "can.h"
    #include "can_board_ids.h"
    #include "log.h"
    #include "new_can_setters.h"
    #include "tasks.h"

    void wait_tasks(int t)
    {
       for (size_t i = 0; i < t; ++i)
          wait(&s_end_task_sem);
    }

    TASK(master_task, TASK_MIN_STACK_SIZE) {
       int counter = 0;
       while (true) {
         run_fast_cycle();
         if (!(counter%10))
            run_medium_counter();
         if (!(counter%100))
            run_slow_counter();
         vTaskDelay(pdMS_TO_TICKS(1000));
         ++counter;
       }
    }

    void run_fast_cycle()
    {
      run_critical_tasks();
      wait_tasks(1);
    }

    void run_medium_cycle()
    {
      run_can_rx_cycle();
      wait_tasks(1);
      run_gpio_cycle();
      run_test_cycle();
      run_help_cycle();
      wait_tasks(3);
      run_last_cycle();
      wait_tasks(1);
      run_can_tx_cycle();
      wait_tasks(1);
    }

    void run_slow_cycle()
    {

    }

    int main() {
      log_init();
      LOG_DEBUG("Welcome to TEST!");

      tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

      tasks_start();

      LOG_DEBUG("exiting main?");
      return 0;
    }

    """

DEFAULT_DEPS = ["FreeRTOS", "ms-common"]

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
        main_file.write(textwrap.dedent(MAIN_TEMPLATE))

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
