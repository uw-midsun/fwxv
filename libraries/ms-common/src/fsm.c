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
  if (!fsm->transition_table[fsm->curr_state * fsm->num_states + to]) {
    LOG_DEBUG("Transition from State %d -> State %d does not exist\n", fsm->curr_state, to);
    return STATUS_CODE_INTERNAL_ERROR;
  }
  fsm->curr_state = to;
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
      self->states[self->curr_state].inputs(self, self->context);
      // If transition has occurred, execute output function
      if (self->transitioned) {
        self->states[self->curr_state].outputs(self->context);
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
    fsm->curr_state = settings->initial_state;
  }
  fsm->transition_table = settings->transitions;
  fsm->states = settings->state_list;
  fsm->fsm_sem = xSemaphoreCreateCountingStatic(CYCLE_RX_MAX, 0, &fsm->sem_buf);
  return STATUS_CODE_OK;
}
