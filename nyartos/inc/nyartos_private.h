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

#ifndef NYARTOS_PRIVATE_H
#define NYARTOS_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* ------------------------------------------------------------------------------ */
/* Includes */
/* ------------------------------------------------------------------------------ */

#include "nyartos.h"
#include "nyartos_port.h"
#include "nyartos_system_config.h"
#include "nyartos_task_config.h"
#include "nyartos_private_port.h"

/* ------------------------------------------------------------------------------ */
/* Helper Macros */
/* ------------------------------------------------------------------------------ */

#define NYA_ARRAY_SIZE(_array)    (sizeof(_array) / sizeof(_array[0]))
#define NYA_ASSERT(_condition, _id)    do { if (!(_condition)) nya_panic((_id)); } while(0);

/* ------------------------------------------------------------------------------ */
/* Types */
/* ------------------------------------------------------------------------------ */

/**
 * @brief Type for task entry functions.
 */
typedef void (*nya_task_func_t)(void *);

/**
 * @brief Wait return type.
 */
typedef enum
{
    NYA_WAIT_RET_OK,
    NYA_WAIT_RET_TIMEOUT,
} nya_wait_ret_t;

/**
 * @brief Task states.
 */
typedef enum
{
    NYA_TASK_READY,
    NYA_TASK_RUNNING,
    NYA_TASK_ASLEEP,
    NYA_TASK_WAITING_FOR_EVENT,
} nya_task_state_t;

typedef enum
{
    NYA_EVENT_UNINITIALIZED = 0,
    NYA_EVENT_MUTEX,
    NYA_EVENT_SEMAPHORE,
    NYA_EVENT_TOP,
} nya_event_type_t;

/**
 * @brief Queue type.
 */
typedef struct
{
    struct nya_tcb_t *first;    /**< First task in this queue. */
    struct nya_tcb_t *last;     /**< Last task in this queue. */
} nya_queue_t;

/**
 * @brief Event type.
 */
typedef struct
{
    nya_event_type_t type;
    struct nya_tcb_t *holder;
    nya_queue_t queue;
    nya_size_t count;
} nya_event_t;

/**
 * @brief Task control block type.
 */
typedef struct nya_tcb_t
{
    nya_stack_t *stack_ptr;

    nya_size_t delay;
    nya_task_state_t state;
    nya_task_id_t tid;

    nya_u8_t base_prio;                /**< Base priority */
    nya_u8_t curr_prio;                /**< Highest inherited priority */

    struct nya_tcb_t *next;            /**< Next task in event queue */
    struct nya_tcb_t *prev;            /**< Previous task in event queue */
    nya_event_t *wait_event;           /**< The event that this task is waiting for */
    nya_wait_ret_t wait_return;        /**< This indicates if task timed out while waiting for something */

#if NYA_CFG_ENABLE_MESSAGE_QUEUES
    nya_msgq_t msgq;
#endif /* if NYA_CFG_ENABLE_MESSAGE_QUEUES */

#if NYA_CFG_ENABLE_STATS
    nya_stack_t *stack_end;
    nya_size_t stack_size;
#endif /* if NYA_CFG_ENABLE_STATS */
} nya_tcb_t;

/**
 * @brief System context type.
 */
typedef struct
{
    nya_bool_t is_running;

    nya_tcb_t tcbs[NYA_TASK_ID_CNT];
    nya_event_t events[NYA_EVENT_ID_CNT];
    nya_queue_t priorities[NYA_PRIORITY_LEVEL_CNT];

    nya_u32_t ready_priorities;
    nya_u8_t isr_nesting_cnt;
} nya_os_ctx_t;

/* ------------------------------------------------------------------------------ */
/* Globals */
/* ------------------------------------------------------------------------------ */

extern nya_tcb_t *volatile nya_curr_task;
extern nya_tcb_t *volatile nya_next_task;
extern nya_os_ctx_t os_ctx;

/* ------------------------------------------------------------------------------ */
/* Internal System Prototypes */
/* ------------------------------------------------------------------------------ */

/**
 * @brief This function is called when a task exits.
 */
void nya_task_exit(void);

/**
 * @brief    Triggers a context switch.
 * @warning  Don't call it from an ISR, @c nya_exit_isr() handles context switching from ISRs.
 */
void nya_schedule(void);

/**
 * @brief   Increments the systick.
 * @note    This function contains a critical section.
 */
void nya_systick(void);

/**
 * @brief Initializes a mutex.
 * @param [in] id - id of the mutex
 */
void nya_event_init(nya_event_id_t id,
                    nya_event_type_t type,
                    nya_u32_t count);

/**
 * @brief Times out a task and removes it from an event waiting list.
 * @param [in] task - timed out task
 */
void nya_event_timeout(nya_tcb_t *task);

/**
 * @brief   Pushes a task to a queue.
 * @note    Call this from within a critical section.
 */
void nya_queue_push(nya_tcb_t *task, nya_queue_t *queue);

/**
 * @brief   Pops a queue.
 * @note    Call this from within a critical section.
 */
void nya_queue_pop(nya_queue_t *queue);

/**
 * @brief   Removes a task from a queue.
 * @note    Call this from within a critical section.
 */
void nya_queue_remove(nya_tcb_t *task, nya_queue_t *queue);

/**
 * @brief   Updates a task priority.
 */
void nya_update_priority(nya_tcb_t *task, nya_u8_t new_prio);

/**
 * @brief   Initializes a task's stack.
 * @note    It needs to be implemented in @c nya_port.c or @c nya_port.s
 * @note    If the stack needs to be aligned, it should be handled here.
 *
 * @param[in] entry_func - pointer to the task's entry function
 * @param[in] stack_ptr - pointer to the beginning of the task's stack
 * @param[in] stack_size - size of the stack
 * @param[in] param - task entry function parameter
 * @return nya_stack_t* - pointer of the initialized stack
 */
nya_stack_t* nya_port_init_stack(nya_task_func_t entry_func,
                                 nya_stack_t *stack_ptr,
                                 nya_stack_t stack_size,
                                 void *param);

/**
 * @brief Initializes the system and starts scheduling.
 */
void nya_port_startup(void);

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYARTOS_PRIVATE_H */
