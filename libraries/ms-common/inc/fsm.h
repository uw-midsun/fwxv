#pragma once
// Fsm.h

// Module used to implement finite-state-machine behaviour in a
// freeRTOS environment. If you are not familiar with FSMs as a
// concept, please see https://uwmidsun.atlassian.net/l/c/wPB3ovQP
// For a demo of the core functionality, see fwxv/projects/fsm_demo
//
// Functionality:
//  FSM operation is regulated by cycles. Each time a cycle is run,
//  via a call to fsm_run_cycle(), the fsm will execute the user-
//  written input function to determine if a transition should take
//  place (a call to fsm_transition() occures. These transitions
//  must have a corresponding entry in the transition list, otherwise
//  an error will occur. Output functions associated with a state are
//  executed when that state is transitioned into.
//
// Usage:
// Forward declare an FSM in the header file:
//    DECLARE_FSM(fsm_name)
//
// Define a FSM object and associated task with a set number of states:
//    #define NUM_STATES X
//    FSM(fsm_name, NUM_STATES)
//
// Declare input functions with transitions, and output functions:
//    static void prv_input1(Fsm *fsm, void *context) {
//    ...
//     fsm_transition(TO_STATE);
//    }
//    static void prv_output1(Fsm *fsm, void *context) {
//    ...
//    }
//
// Declare states in list:
//    static FsmStateList s_list[NUM_STATES] = {
//      STATE(state_id, input_func, output_func),
//      ...
//    }
//
// Declare transitions in list:
//    static FsmTransition s_transitions[NUM_TRANSITIONS] {
//      TRANSITION(from, to),
//      ...
//    }
//
// Initialize the FSM:
//    FsmSettings settings = {
//      .state_list = s_list,
//      .transitions = s_transitions,
//      .num_transitions = NUM_TRANSITIONS,
//      .initial_state = INITIAL_STATE,
//    };
//    fsm_init(fsm_name, settings, context);
//
// The FSM will now commence operation when the scheduler is started
// To communicate with the fsm, ie send a notification, simply use
// the name supplied upon creation:
//    notify(fsm_name, notify_event);

#include "fsm_impl.h"
#include "notify.h"
#include "semphr.h"
#include "status.h"
#include "tasks.h"

#define FSM_PRIORITY 1
// TODO(mitchellostler): make user defined
#define FSM_TASK_STACK TASK_STACK_1024
#define FSM_TIMEOUT_MS 1000
// TODO(mitchellostler): make this defined as part of project API
#define CYCLE_RX_MAX 15

struct Fsm;
typedef uint8_t StateId;

typedef void (*StateInputFunc)(struct Fsm *fsm, void *context);
typedef void (*StateOutputFunc)(void *context);

typedef struct FsmState {
  StateId id;               // Unique State ID - Should be based on enumerated type
  StateInputFunc inputs;    // Function used to parse inputs and transition if needed
  StateOutputFunc outputs;  // Output function associated with state
} FsmState;

typedef struct Fsm {
  FsmState *curr_state;
  const uint8_t num_states;
  FsmState **transition_table;
  SemaphoreHandle_t fsm_sem;
  StaticSemaphore_t sem_buf;
  bool transitioned;
  void *context;
} Fsm;

typedef struct FsmTransition {
  StateId from;
  StateId to;
} FsmTransition;

typedef struct FsmSettings {
  FsmState *state_list;
  FsmTransition *transitions;
  uint8_t num_transitions;
  StateId initial_state;
} FsmSettings;

// Forward declares an extern pointer to a task of name "fsm_name"
#define DECLARE_FSM(name) _DECLARE_FSM(name)

// Creates the FSM structure with the supplied number of states
// and initializes its associated FSM task
// num_states must be a defined constant
#define FSM(name, num_fsm_states) _FSM(name, num_fsm_states)

// Creates state with associated id in a state list
// State id must be unique (preferred to use enum type)
#define STATE(state_id, input_func, output_func) _STATE(state_id, input_func, output_func)

// Defines a transition from StateId "from" -> StateId "to"
// Must be declared in a transition list
#define TRANSITION(from_state, to_state) _TRANSITION(from_state, to_state)

#define fsm_init(fsm, settings, context) _fsm_init(fsm, settings, context)

// Initiates a transition from the current state
// Transition must exist in transition table
StatusCode fsm_transition(Fsm *fsm, StateId to);

// Executes an FSM cycle
// Should be called with name supplied to FSM()
void fsm_run_cycle(Task *fsm);

// Internal implementation to initialize FSM
// Do not call directly. Use the fsm_init() macro above
StatusCode _init_fsm(Fsm *fsm, FsmSettings *settings, void *context);

// Fsm task function implementation - Do not call directly
void _fsm_task(void *context);
