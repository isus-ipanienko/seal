/*
 * NYARTOS - A Real Time Operating System For Embedded Devices
 * Copyright (c) 2022 Paweł Religa
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NYA_API_H
#define NYA_API_H

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* ------------------------------------------------------------------------------ */
/* Includes */
/* ------------------------------------------------------------------------------ */

#include "nyartos_port.h"
#include "nyartos_task_config.h"

/* ------------------------------------------------------------------------------ */
/* Macros */
/* ------------------------------------------------------------------------------ */

/*TODO: implement time management macros */
#define NYA_HOURS_TO_TICKS(_hours)    (_hours)
#define NYA_MINS_TO_TICKS(_mins)      (_mins)
#define NYA_SECS_TO_TICKS(_secs)      (_secs)
#define NYA_MS_TO_TICKS(_ms)          (_ms)

#define NYA_UNUSED(_param)            _param = _param;

/* ------------------------------------------------------------------------------ */
/* Types */
/* ------------------------------------------------------------------------------ */

typedef enum
{
    NYA_OK = 0,
    NYA_ERROR,
    NYA_WRONG_EVENT,
    NYA_TIMEOUT,
} nya_error_t;

/* ------------------------------------------------------------------------------ */
/* API Prototypes */
/* ------------------------------------------------------------------------------ */

/**
 * @brief Call this function when entering a kernel aware ISR.
 */
void nya_enter_isr(void);

/**
 * @brief Call this function when exiting a kernel aware ISR.
 */
void nya_exit_isr(void);

/**
 * @brief Call this function in main() to initialize and start the OS.
 */
void nya_init(void);

/**
 * @brief   This function delays a task for a specified amount of systicks.
 * @note    It's non-blocking.
 * @note    Use NYA_MS_TO_TICKS(), NYA_SECS_TO_TICKS(), NYA_MINS_TO_TICKS() and
 *          NYA_HOURS_TO_TICKS() to convert from units of time to systicks.
 *
 * @param   ticks - Current task will be delayed for this many ticks.
 */
void nya_sleep(nya_size_t ticks);

/**
 * @brief   Attempts to take a mutex.
 *          @warning Waiting on a semaphore while holding any amount of mutexes may introduce priority inversion.
 *          @note timeout == 0 indicates that the task is willing to wait indefinitely
 * @param   [in] id - id of the mutex
 * @param   [in] timeout - timeout in systicks
 * @return  NYA_OK - mutex taken successfully
 */
nya_error_t nya_mutex_take(nya_event_id_t id,
                           nya_size_t timeout);

/**
 * @brief   Attempts to give a mutex.
 * @param   [in] id - id of the mutex
 * @return  NYA_OK - mutex given successfully
 */
nya_error_t nya_mutex_give(nya_event_id_t id);

/**
 * @brief   Attempts to take a semaphore.
 *          @warning Waiting on a semaphore while holding any amount of mutexes may introduce priority inversion.
 *          @note timeout == 0 indicates that the task is willing to wait indefinitely
 * @param   [in] id - id of the semaphore
 * @param   [in] timeout - timeout in systicks
 * @return  NYA_OK - semaphore taken successfully
 */
nya_error_t nya_semaphore_take(nya_event_id_t id,
                               nya_size_t timeout);

/**
 * @brief   Attempts to give a semaphore.
 * @param   [in] id - id of the semaphore
 * @return  NYA_OK - semaphore given successfully
 */
nya_error_t nya_semaphore_give(nya_event_id_t id);

/* ------------------------------------------------------------------------------ */
/* Hooks */
/* ------------------------------------------------------------------------------ */

/**
 * @brief This hook is called once upon entering a kernel panic.
 */
void nya_panic_hook(void);

/**
 * @brief This hook is called once upon exiting a task.
 */
void nya_task_exit_hook(void);

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif // ifndef NYA_API_H
