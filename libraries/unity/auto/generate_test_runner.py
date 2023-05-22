from sys import argv
import re

def generate(input_file: str, includes: list, tests: list):
    return f"""
/* AUTOGENERATED FILE. DO NOT EDIT. */
/*=======Automagically Detected Files To Include=====*/
{ IF_TEST_IN_TASK(tests, '''
#include "FreeRTOS.h"
#include "tasks.h"
''') }
#include "unity.h"
#include <stdlib.h>

/*=======External Functions This Runner Calls=====*/
extern void setup_test(void);
{ '@nl@'.join( [ f'extern void { test[0] }(void);' for test in tests ] ) }

/*=======Test Runner Used To Run Each Test=====*/
static void run_test(UnityTestFunction func, const char *name, UNITY_LINE_TYPE line_num) @co@
  Unity.CurrentTestName = name;
  Unity.CurrentTestLineNumber = line_num;

  Unity.NumberOfTests++;
  UNITY_CLR_DETAILS();

  if (TEST_PROTECT()) @co@
{ IF_TEST_IN_TASK(tests, '''
    vTaskSuspendAll();
    setup_test();
    xTaskResumeAll();
''', '''
    setup_test();
''') }

    func();
  @cc@

  UnityConcludeTest();
@cc@

/*=======PRE TASK TESTS=====*/
void test_pre_task(void) @co@
{ '@nl@'.join([ f'  run_test({ test[0] }, "{ test[0] }", { test[1] });' for test in tests if test[2] == "TEST_PRE_TASK" ]) }
@cc@

{ IF_TEST_IN_TASK(tests, f'''
/*=======IN TASK TESTS=====*/
TASK(test_task, TASK_STACK_1024) @co@
  UnityBegin("{ input_file }");

{ '@nl@'.join([ f'  run_test({ test[0] }, "{ test[0] }", { test[1] });' for test in tests if test[2] == "TEST_IN_TASK" ]) }

  exit(UnityEnd());
@cc@
''')}

/*=======MAIN=====*/
int main() @co@
  setup_test();
  test_pre_task();
{ IF_TEST_IN_TASK(tests, '''
  tasks_init();
  tasks_init_task(test_task, configMAX_PRIORITIES - 1, NULL);
  tasks_start(); 
''')}
  return 0;
@cc@
"""


def IF_TEST_IN_TASK(tests: list, output: str, otherwise: str = ''):
  return has_type(tests, "TEST_IN_TASK", output, otherwise)
def IF_TEST_PRE_TASK(tests: list, output: str, otherwise: str = ''):
  return has_type(tests, "TEST_PRE_TASK", output, otherwise)

def has_type(tests: list, type: str, output: str, otherwise: str = ''):
  if (any(test[2] == type for test in tests )):
    return output
  else:
    return otherwise

def clean_input(input):
  '''
  [UNIMPLEMENTED] Remove all C style comments from input
  '''
  pass

def get_tests(input):
  '''
  Get all tests in file, return (test_name, line_number, type)[]
  '''
  tests = []
  test_types = { "TEST_PRE_TASK", "TEST_IN_TASK" }
  default_type = "TEST_PRE_TASK"
  for lineNum, line in enumerate(input):
    res = re.search('^\s*void \s*(test_([A-z0-9]|_)*)\s*\((void)?\)', line)
    if res != None:
      test_type = input[lineNum - 1].strip() if (lineNum > 0) else ''
      tests.append((res.group(1), lineNum + 1, test_type if (test_type in test_types) else default_type))
  return tests

def get_includes(input):
  '''
  Get all includes needed
  '''
  includes = [ "FreeRTOS.h", "unity.h", "stdlib.h", "tasks.h" ]
  # for line in input:
  #   if (line == "TEST_IN_TASK"):
  #     includes.append("tasks.h")
  #     return includes
  return includes

def main(input_file, output_file):
  # find includes etc.
  replace = [ ('@co@', '{'), ('@cc@', '}'), ('@nl@', '\n') ]

  with open(input_file, 'r') as input:
    content = input.readlines()

  clean_input(content)

  include = get_includes(content)
  tests = get_tests(content)

  if len(tests) == 0:
    raise Exception(f"No tests specified in {input_file}")

  with open(output_file, 'w') as output:
    res = generate("test", include, tests)
    for k, v in replace:
      res = res.replace(k, v)
    output.write(res)

main(argv[2], argv[3])
