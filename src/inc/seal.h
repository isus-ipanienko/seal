#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "port.h"
#include "config.h"

/*TODO: implement time management macros */
#define OS_HOURS_TO_TICKS(_hours) (_hours)
#define OS_MINS_TO_TICKS(_mins) (_mins)
#define OS_SECS_TO_TICKS(_secs) (_secs)
#define OS_MS_TO_TICKS(_ms) (_ms)

#define OS_UNUSED(_param) _param = _param;

typedef enum {
  OS_OK = 0,
  OS_ERROR,
  OS_NULL_PARAM,
  OS_WRONG_EVENT,
  OS_TIMEOUT,
  OS_EVENT_INITIALIZED,
  OS_TASK_EXITED,
  OS_STARTUP_EXITED,
  OS_ISR_OVERFLOW,
  OS_ISR_UNDERFLOW
} os_error_t;

/**
 * @brief Call this function when entering a kernel aware ISR.
 */
void os_enter_isr(void);

/**
 * @brief Call this function when exiting a kernel aware ISR.
 */
void os_exit_isr(void);

/**
 * @brief Call this function in main() to initialize and start the OS.
 */
void os_init(void);

/**
 * @brief   This function delays a task for a specified amount of systicks.
 * @note    It's non-blocking.
 * @note    Use OS_MS_TO_TICKS(), OS_SECS_TO_TICKS(), OS_MINS_TO_TICKS() and
 *          OS_HOURS_TO_TICKS() to convert from units of time to systicks.
 *
 * @param   ticks - Current task will be delayed for this many ticks.
 */
void os_sleep(os_size_t ticks);

/**
 * @brief   Attempts to take a mutex.
 *          @warning Waiting on a semaphore while holding any amount of mutexes
 *                   may introduce priority inversion.
 *          @note timeout == 0 indicates that the task is willing to wait
 *                indefinitely
 * @param   [in] id - id of the mutex
 * @param   [in] timeout - timeout in systicks
 * @return  OS_OK - mutex taken successfully
 */
os_error_t os_mutex_take(os_event_id_t id, os_size_t timeout);

/**
 * @brief   Attempts to give a mutex.
 * @param   [in] id - id of the mutex
 * @return  OS_OK - mutex given successfully
 */
os_error_t os_mutex_give(os_event_id_t id);

/**
 * @brief   Attempts to take a semaphore.
 *          @warning Waiting on a semaphore while holding any amount of mutexes
 *                   may introduce priority inversion.
 *          @note timeout == 0 indicates that the task is willing to wait
 *                indefinitely
 * @param   [in] id - id of the semaphore
 * @param   [in] timeout - timeout in systicks
 * @return  OS_OK - semaphore taken successfully
 */
os_error_t os_semaphore_take(os_event_id_t id, os_size_t timeout);

/**
 * @brief   Attempts to give a semaphore.
 * @param   [in] id - id of the semaphore
 * @return  OS_OK - semaphore given successfully
 */
os_error_t os_semaphore_give(os_event_id_t id);

/**
 * @brief This function is called when something really bad happens.
 */
void os_panic(os_error_t reason);

/**
 * @brief This hook is called once upon entering a kernel panic.
 */
void os_panic_hook(os_error_t reason);

/**
 * @brief This hook is called once upon exiting a task.
 */
void os_task_exit_hook(void);

#ifdef __cplusplus
}
#endif
