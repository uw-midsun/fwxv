#!/usr/bin/env python

import os

def preprocessor_filter(filename, source_file):
    """
    A SCons builder that filters preprocessor output to strip out #include'd file content.
    We also strip out the linemarkers output by the C preprocessor.
    `filename` is the name of the base file whose content we want to preserve.
    `source_file` is the file to read from (the SCons `source` parameter will be ignored).
    Note that `source_file` will be deleted at the end (it should be a temp file).
    """

    def action(target, source, env):
        output_lines = []
        with open(source_file, 'r') as file:
            # The C preprocessor will output 'linemarkers' of the following form:
            #     # <lineno> "<source filename>" <id>...
            # We want to keep only lines after linemarkers from the given filename.
            # See https://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html.
            in_file = False
            for line in file:
                if line.startswith('# '):
                    # We're in a linemarker - naively test if it's for the current file
                    in_file = '"' + filename + '"' in line
                elif in_file:
                    output_lines.append(line)

        with open(target[0].path, 'w') as output_file:
            output_file.writelines(output_lines)

        # delete the temp file we were given
        os.remove(source_file)

        return 0

    return action
