#include <stdint.h>

#include "FreeRTOS.h"

// These are needed to statically allocate the memory for the idle and timer tasks. We have
// configSUPPORT_STATIC_ALLOCATION set, and so the application must provide implementations of
// vApplicationGetIdleTaskMemory and vApplicationGetTimerTaskMemory.
// See https://www.freertos.org/a00110.html, configSUPPORT_STATIC_ALLOCATION section.

#if (configSUPPORT_STATIC_ALLOCATION == 1)

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  // Pass out a pointer to the StaticTask_t structure in which the Idle task's
  // state will be stored.
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  // Pass out the array that will be used as the Idle task's stack.
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if (configUSE_TIMERS == 1)

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  // Very similar to vApplicationGetIdleTaskMemory, but for the timer task.
  // Note that the stack depth is configTIMER_TASK_STACK_DEPTH, which can be modified in
  // FreeRTOSConfig.h.
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#endif

#endif
