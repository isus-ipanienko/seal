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

/* ------------------------------------------------------------------------------ */
/* Types */
/* ------------------------------------------------------------------------------ */

/**
 * @brief Task states.
 */
typedef enum
{
    NYA_TASK_RUNNING,
    NYA_TASK_PREEMPTED,
    NYA_TASK_WAITING,
} nya_task_state_t;

typedef enum
{
    NYA_MUTEX_T,
    NYA_MSGQ_T,
} nya_event_type_t;

/**
 * @brief Event type.
 */
typedef struct
{
    nya_event_type_t type;
    struct nya_tcb_t *holder;
} nya_event_t;

/**
 * @brief Task control block type.
 */
typedef struct nya_tcb_t
{
    nya_stack_t *stack_ptr;

    nya_size_t delay;
    nya_task_state_t state;

    nya_u8_t base_prio;               /**< Base priority */
    struct nya_tcb_t *next_in_prioq;  /**< Next task in priority queue */
    struct nya_tcb_t *prev_in_prioq;  /**< Previous task in priority queue */

    nya_u8_t high_prio;               /**< Highest inherited priority */
    struct nya_tcb_t *next_in_waitq;  /**< Next task in waiting queue */
    struct nya_tcb_t *prev_in_waitq;  /**< Previous task in waiting queue */
    nya_event_t *wait_event;          /**< The event that this task is waiting for */
    nya_event_type_t wait_event_type; /**< Type of the event that this task is waiting for */

#if NYA_CFG_ENABLE_MESSAGE_QUEUES
    nya_msgq_t msgq;
#endif /* if NYA_CFG_ENABLE_MESSAGE_QUEUES */

#if NYA_CFG_ENABLE_STATS
    nya_stack_t *stack_end;
    nya_size_t stack_size;
#endif /* if NYA_CFG_ENABLE_STATS */
} nya_tcb_t;

/**
 * @brief Stores information about a single priority level.
 */
typedef struct
{
    nya_tcb_t *first;
    nya_tcb_t *last;
    nya_size_t count; /**< Number of waiting tasks in this priority. */
    nya_u8_t mode;    /**< For future use. */
} nya_prioq_t;

/**
 * @brief System context type.
 */
typedef struct
{
    nya_tcb_t *curr_task;
    nya_tcb_t *next_task;

    nya_bool_t is_running;

    nya_tcb_t tcb_l[NYA_CFG_TASK_CNT];
    nya_event_t event_l[NYA_CFG_KERNEL_EVENT_CNT];
    nya_prioq_t prioq_l[NYA_CFG_PRIORITY_LEVELS];
    nya_u8_t prio_grp_rdy[8];
    nya_u8_t prio_grp_cluster_rdy;

    nya_u8_t isr_nesting_cnt;

    const nya_u8_t resolve_prio_lkp[256];
    const nya_u8_t prio_indx_lkp[64];
    const nya_u8_t prio_mask_lkp[64];
} nya_os_ctx_t;

/* ------------------------------------------------------------------------------ */
/* Globals */
/* ------------------------------------------------------------------------------ */

extern nya_os_ctx_t os_ctx;

/* ------------------------------------------------------------------------------ */
/* Global Prototypes */
/* ------------------------------------------------------------------------------ */

/**
 * @brief   Pops a priority queue.
 * @note    Doesn't support priority boosting, yet.
 *          TODO: add temp priority field to tcbs
 * @note    Always call this from within a critical section.
 */
void nya_priority_pop(nya_u8_t priority);

/**
 * @brief   Pushes a task to a priority queue.
 * @note    Doesn't support priority boosting, yet.
 *          TODO: add temp priority field to tcbs
 * @note    Always call this from within a critical section.
 */
void nya_priority_push(nya_size_t id,
                       nya_u8_t priority);

nya_bool_t nya_scheduler_set_next_task(void);
void nya_scheduler_switch(void);

void nya_time_systick(void);

nya_stack_t* nya_port_init_stack(nya_task_func_t entry_func,
                                 nya_stack_t *stack_ptr,
                                 nya_stack_t stack_size);

void nya_port_startup(void);

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYARTOS_PRIVATE_H */
