from sys import argv
import re
import jinja2
import os

def get_tests(input):
  '''
  Get all tests in file, return a list of tests, each test is
  {
    "name": <str>, name of the test
    "line": <int>, the line number of the test
    "type": <str>, "TEST_PRE_TASK" or "TEST_IN_TASK"
  }
  '''
  tests = []
  test_types = { "TEST_PRE_TASK", "TEST_IN_TASK" }
  default_type = "TEST_PRE_TASK"
  for lineNum, line in enumerate(input):
    res = re.search('^\s*void \s*(test_([A-z0-9]|_)*)\s*\((void)?\)', line)
    if res != None:
      test_type = input[lineNum - 1].strip() if (lineNum > 0) else ''
      tests.append({
        "name": res.group(1), 
        "line": lineNum + 1, 
        "type": test_type if (test_type in test_types) else default_type
      })
  return tests

def main(input_file, output_file):
  current_dir = os.path.dirname(os.path.abspath(__file__))
  template_loader = jinja2.FileSystemLoader(searchpath=current_dir)
  env = jinja2.Environment(loader=template_loader)

  with open(input_file, 'r') as input:
    content = input.readlines()

  tests = get_tests(content)
  data = {
    "tests": tests,
    "has_in_task_test": any([test["type"] == "TEST_IN_TASK" for test in tests]),
    "filename": input_file,
  }

  if len(tests) == 0:
    raise Exception(f"No tests specified in {input_file}")

  with open(output_file, 'w') as output:
    res = env.get_template("test_runner.c.jinja").render(data=data)
    output.write(res)

main(argv[2], argv[3])
