#pragma once

// Internal implementations of the macros in tasks.h. Do not use directly.

// Tasks are implemented as global Task variables with the same name which store
// info needed to start the task.

#define _DECLARE_TASK(task_name) extern Task task_name

// Initialize everything the task needs.
// We initialize the stack here: the arm linker will error if too much stack space is used.
#define _TASK(task_name, task_stack_size)                        \
  /* forward declaration so we can reference it in the Task */   \
  static void _prv_task_impl_##task_name(void *);                \
  static StackType_t _s_stack_##task_name[task_stack_size];      \
  Task task_name = {                                             \
    .task_func = _prv_task_impl_##task_name,                     \
    .name = #task_name,                                          \
    .stack = _s_stack_##task_name,                               \
    .stack_size = task_stack_size,                               \
    .handle = NULL, /* will be initialized by tasks_init_task */ \
  };                                                             \
  static void _prv_task_impl_##task_name(void *context)
