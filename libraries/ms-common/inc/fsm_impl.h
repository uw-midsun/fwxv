#pragma once
// Internal Fsm Implementation
// Do not call any of these functions directly
// Use API defined in Fsm.h

// Internal Implementation of FSM initialization
// Declares a FSM state table, 

#define _DECLARE_FSM(name) \
  DECLARE_TASK(name);      \
  extern Fsm *name##_fsm


// Internal fsm declaration
// Creates square transition table and task with name given to FSM
// The _fsm_task function receives the FSM as its context
#define _FSM(name, num_fsm_states)                                \
  static FsmState *name##_table[num_fsm_states * num_fsm_states]; \
  Fsm *name##_fsm = &((Fsm){                                      \
      .transition_table = name##_table,                           \
      .num_states = num_fsm_states,                               \
  });                                                             \
  TASK(name, TASK_STACK_512) {                                    \
    _fsm_task(context);                                           \
  }

#define _STATE(state_id, input_func, output_func) \
  [state_id] = {  \
    .id = state_id, .inputs = input_func, \
    .outputs = output_func  \
  }


// Internal Transition implementation
#define _TRANSITION(from_state, to_state) \
  { .from = from_state, .to = to_state }

// Initialize the fsm task and fsm object
// Named fsm object passed as context to task
// Context above passed to _init_fsm, stored in FSM object
#define _fsm_init(fsm, settings, context)            \
  tasks_init_task(fsm, TASK_PRIORITY(FSM_PRIORITY), fsm##_fsm); \
  _init_fsm(fsm##_fsm, &settings, context)

