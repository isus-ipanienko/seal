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

/* ------------------------------------------------------------------------------ */
/* Includes */
/* ------------------------------------------------------------------------------ */

#include "nyartos_private.h"

/* ------------------------------------------------------------------------------ */
/* Context */
/* ------------------------------------------------------------------------------ */

typedef struct
{
    nya_tcb_t tcb[NYA_CFG_TASK_CNT];
    nya_tcb_t *next_tcb_in_priority[NYA_CFG_PRIORITY_LEVELS];
    nya_tcb_t *last_tcb_in_priority[NYA_CFG_PRIORITY_LEVELS];
    nya_u8_t priority_group_ready_tasks[NYA_CFG_PRIORITY_LEVELS];
    nya_u8_t priority_group_ready[8];
    nya_u8_t priority_group_cluster_ready;
    const nya_u8_t ready_to_index_lookup[256];
} evsched_ctx_t;

static evsched_ctx_t ctx =
{
    .ready_to_index_lookup =
    {
        0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    }
};

/* ------------------------------------------------------------------------------ */
/* Private Prototypes */
/* ------------------------------------------------------------------------------ */

static void _task_switch(void);
void nya_init();

/* ------------------------------------------------------------------------------ */
/* Private Declarations */
/* ------------------------------------------------------------------------------ */

static void _task_switch(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    /*TODO: task switching */

    NYA_EXIT_CRITICAL();
}

void _init_tcb(nya_u16_t index,
               nya_u8_t priority)
{
    ctx.tcb[index].priority = priority;

    if (ctx.next_tcb_in_priority[priority] == 0)
    {
        ctx.next_tcb_in_priority[priority] = &ctx.tcb[index];
        ctx.last_tcb_in_priority[priority] = &ctx.tcb[index];
    }
    else
    {
        ctx.last_tcb_in_priority[priority]->next_in_priority_group = &ctx.tcb[index];
        ctx.last_tcb_in_priority[priority] = &ctx.tcb[index];
    }
}

void nya_init()
{
    nya_u16_t index = 0;

#define NYA_TASK(_priority) \
    _init_tcb(index++,      \
              _priority);
    NYA_TASK_DEFINITIONS
#undef NYA_TASK
}
