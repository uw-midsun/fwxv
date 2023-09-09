#include "fsm.h"

#include <limits.h>
#include <stdio.h>

#include "log.h"
#include "notify.h"

StatusCode fsm_transition(Fsm *fsm, StateId to) {
  if (fsm == NULL || to >= fsm->num_states) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // Check entry in table to see if the transition exists
  FsmState *next_state = fsm->transition_table[fsm->curr_state->id * fsm->num_states + to];
  if (next_state == NULL) {
    LOG_DEBUG("Transition from State %d -> State %d does not exist\n", fsm->curr_state->id, to);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  fsm->curr_state = next_state;
  fsm->transitioned = true;
  return STATUS_CODE_OK;
}

void fsm_run_cycle(Task *fsm) {
  if (fsm == NULL || fsm->context == NULL || ((Fsm *)fsm->context)->fsm_sem == NULL) {
    LOG_CRITICAL("No fsm configured, cannot run cycle\n");
    return;
  }
  Fsm *task_fsm = fsm->context;
  // Allow fsm to run
  BaseType_t ret = xSemaphoreGive(task_fsm->fsm_sem);
  if (ret == pdFALSE) {
    LOG_CRITICAL("FSM run Cycle Failed\n");
  }
}

void _fsm_task(void *context) {
  Fsm *self = context;
  BaseType_t ret;
  while (true) {
    ret = xSemaphoreTake(self->fsm_sem, pdMS_TO_TICKS(FSM_TIMEOUT_MS));
    if (ret == pdTRUE) {
      // Parse inputs,updating curr_state if transition occurred
      self->curr_state->inputs(self, self->context);
      // If transition has occurred, execute output function
      if (self->transitioned) {
        self->curr_state->outputs(self->context);
        self->transitioned = false;
      }
    } else {
      // TODO(mitchellostler): Timeout Error handling
      LOG_DEBUG("FSM timeout\n");
    }
    // Indicate that a fsm_cycle is finished
    send_task_end();
  }
}

StatusCode _init_fsm(Fsm *fsm, FsmSettings *settings, void *context) {
  if (fsm == NULL || settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  fsm->context = context;
  if (settings->initial_state > fsm->num_states) {
    return STATUS_CODE_INVALID_ARGS;
  } else {
    fsm->curr_state = &settings->state_list[settings->initial_state];
  }
  fsm->fsm_sem = xSemaphoreCreateCountingStatic(CYCLE_RX_MAX, 0, &fsm->sem_buf);
  for (int t = 0; t < settings->num_transitions; t++) {
    FsmTransition *tr = &settings->transitions[t];
    if (tr->to > fsm->num_states || tr->from > fsm->num_states) {
      return STATUS_CODE_INVALID_ARGS;
    } else {
      // Store the address of state to at row "from" and column "to"
      fsm->transition_table[tr->from * fsm->num_states + tr->to] = &settings->state_list[tr->to];
    }
  }
  return STATUS_CODE_OK;
}
