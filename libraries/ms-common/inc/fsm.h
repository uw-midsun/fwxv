#pragma once
// Fsm.h
// Module used to implement finite-state-machine behaviour
// FSMs are automatically created with their own task which can be referenced
// by the name passed to DECLARE_FSM(). 
// 
#include "fsm_impl.h"
#include "status.h"
#include "tasks.h"
#include "notify.h"
#include "semphr.h"

#define FSM_PRIORITY 1
#define FSM_TASK_STACK TASK_STACK_1024 // TODO: make user defined
#define FSM_TIMEOUT_MS 100
#define CYCLE_RX_MAX 15 // TODO: make this defined as part of project API

struct Fsm;
typedef uint8_t StateId;

typedef void (*StateInputFunc)(struct Fsm *fsm, void *context);
typedef void (*StateOutputFunc)(void *context);

typedef struct FsmState {
  StateId id;              // Unique State ID - Should be based on enumerated type
  StateInputFunc inputs;   // Function used to parse inputs and transition if needed
  StateOutputFunc outputs; // Output function associated with state
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

// Creates an extern pointer to an FSM task of provided name
#define DECLARE_FSM(name) \
  _DECLARE_FSM(name) 

// Creates an FSM of name fsm, and associated state table
#define FSM(name, num_fsm_states) \
  _FSM(name, num_fsm_states) 

// Creates state with associated id in a state list
#define STATE(state_id, input_func, output_func) _STATE(state_id, input_func, output_func) 

// Defines a transition from state "from"->state "to" on event "e"
#define TRANSITION(from_state, to_state) _TRANSITION(from_state, to_state)

#define fsm_init(fsm, settings, context)      \
  _fsm_init(fsm, settings, context)

// Initiates a transition from the current state. Transition must exist in transition table
StatusCode fsm_transition(Fsm *fsm, StateId to);

// Allows FSM to run a cycle - can be called with name of FSM directly
void fsm_run_cycle(Task *fsm);

// Initializes an instance of an FSM. Should not be called directly, 
StatusCode _init_fsm(Fsm *fsm, FsmSettings *settings, void *context);

// Fsm task function implementation 
void _fsm_task(void * context);
