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
#include "nyartos_config.h"
#include "nyartos_private_port.h"

/* ------------------------------------------------------------------------------ */
/* Types */
/* ------------------------------------------------------------------------------ */

/**
 * @brief Task states.
 */
typedef enum
{
    NYA_TASK_RUNNING,
} nya_task_state_t;

typedef void (*nya_task_fun_t)(void *);

/**
 * @brief Task control block type.
 */
typedef struct nya_tcb_t
{
    nya_stack_t *stack_ptr;

#if NYA_CFG_ENABLE_STATS
    nya_stack_t *stack_end;
    nya_size_t stack_size;
#endif /* if NYA_CFG_ENABLE_MESSAGE_QUEUES */

    nya_size_t delay;
    nya_task_state_t state;

#if NYA_CFG_ENABLE_MESSAGE_QUEUES
    nya_msgq_t message_queue;
#endif /* if NYA_CFG_ENABLE_MESSAGE_QUEUES */

    nya_u8_t priority;                            /**< Base priority */
    struct nya_tcb_t *next_in_priority_group;     /**< Required by priority system */
    struct nya_tcb_t *previous_in_priority_group; /**< Required for priority switching */
} nya_tcb_t;

/**
 * @brief System context type.
 */
typedef struct
{
    nya_tcb_t *curr_task;
    nya_tcb_t *next_task;
    nya_tcb_t tcb[NYA_CFG_TASK_CNT];

    nya_tcb_t *first_tcb_in_priority[NYA_CFG_PRIORITY_LEVELS];
    nya_tcb_t *last_tcb_in_priority[NYA_CFG_PRIORITY_LEVELS];
    nya_u8_t priority_group_ready[8];
    nya_u8_t priority_group_cluster_ready;

    nya_u8_t isr_nesting_cnt;

    const nya_u8_t ready_to_index_lookup[256];
    const nya_u8_t priority_to_index_lookup[64];
    const nya_u8_t priority_to_mask_lookup[64];
} nya_sys_ctx_t;

/* ------------------------------------------------------------------------------ */
/* Globals */
/* ------------------------------------------------------------------------------ */

extern nya_sys_ctx_t nya_sys_ctx;

/* ------------------------------------------------------------------------------ */
/* Global Prototypes */
/* ------------------------------------------------------------------------------ */

void nya_task_switch(void);

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYARTOS_PRIVATE_H */
