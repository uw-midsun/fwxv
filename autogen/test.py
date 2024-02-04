import re
from pathlib import Path


def get_data(args):
    output_file = Path(args.output)
    print(output_file)
    input_file = Path(*output_file.parts[3:])
    print(input_file)
    # set args.output to output_dir
    args.output = str(output_file.parent)

    tests = {"TEST_PRE_TASK": [], "TEST_IN_TASK": []}
    default_type = "TEST_PRE_TASK"
    content = input_file.read_text().splitlines()
    for lineNum, line in enumerate(content):
        res = re.search('^\s*void \s*(test_([A-z0-9]|_)*)\s*\((void)?\)', line)
        if res != None:
            test_type = content[lineNum - 1].strip() if (lineNum > 0) else ''
            # append to to correct test type, default to default_type if the type is not valid
            tests.get(test_type, tests[default_type]).append({
                "name": res.group(1),
                "line": lineNum + 1,
            })

    # raise error if there are no tests
    if all([len(tests) == 0 for test in tests.values()]):
        raise Exception(f"No tests specified in {input_file}")

    return {"filename": output_file.stem,
            "pre_task_tests": tests["TEST_PRE_TASK"],
            "in_task_tests": tests["TEST_IN_TASK"]}
