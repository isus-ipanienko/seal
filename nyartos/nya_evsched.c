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
/* System Context */
/* ------------------------------------------------------------------------------ */

nya_sys_ctx_t nya_sys_ctx =
{
    .ready_to_index_lookup    =
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
    },
    .priority_to_index_lookup =
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7
    },
    .priority_to_mask_lookup  =
    {
        1, 2, 4, 8, 16, 32, 64, 128,
        1, 2, 4, 8, 16, 32, 64, 128,
        1, 2, 4, 8, 16, 32, 64, 128,
        1, 2, 4, 8, 16, 32, 64, 128,
        1, 2, 4, 8, 16, 32, 64, 128,
        1, 2, 4, 8, 16, 32, 64, 128,
        1, 2, 4, 8, 16, 32, 64, 128,
        1, 2, 4, 8, 16, 32, 64, 128
    }
};

/* ------------------------------------------------------------------------------ */
/* Private Prototypes */
/* ------------------------------------------------------------------------------ */

static void _init_tcb(nya_u32_t index,
                      nya_u8_t priority,
                      nya_stack_t stack_size);

/* ------------------------------------------------------------------------------ */
/* Private Declarations */
/* ------------------------------------------------------------------------------ */

static void _init_tcb(nya_u32_t index,
                      nya_u8_t priority,
                      nya_stack_t stack_size)
{
    nya_sys_ctx.tcb[index].priority = priority;
#if NYA_CFG_ENABLE_STATS
    nya_sys_ctx.tcb[index].stack_size = stack_size;
#endif /* if NYA_CFG_ENABLE_STATS */

    if (nya_sys_ctx.first_tcb_in_priority[priority] == NYA_NULL)
    {
        nya_sys_ctx.tcb[index].previous_in_priority_group = NYA_NULL;
        nya_sys_ctx.tcb[index].next_in_priority_group = NYA_NULL;
        nya_sys_ctx.first_tcb_in_priority[priority] = &nya_sys_ctx.tcb[index];
        nya_sys_ctx.last_tcb_in_priority[priority] = &nya_sys_ctx.tcb[index];
    }
    else
    {
        nya_sys_ctx.tcb[index].previous_in_priority_group = nya_sys_ctx.last_tcb_in_priority[priority];
        nya_sys_ctx.tcb[index].next_in_priority_group = NYA_NULL;
        nya_sys_ctx.last_tcb_in_priority[priority]->next_in_priority_group = &nya_sys_ctx.tcb[index];
        nya_sys_ctx.last_tcb_in_priority[priority] = &nya_sys_ctx.tcb[index];
    }

    nya_sys_ctx.priority_group_ready[nya_sys_ctx.priority_to_index_lookup[priority]] |=
        nya_sys_ctx.priority_to_mask_lookup[priority];
    nya_sys_ctx.priority_group_cluster_ready |=
        nya_sys_ctx.priority_to_mask_lookup[nya_sys_ctx.priority_to_index_lookup[priority]];

    if (nya_sys_ctx.next_task)
    {
        nya_sys_ctx.next_task = nya_sys_ctx.curr_task->priority < priority ? nya_sys_ctx.next_task :
                                nya_sys_ctx.first_tcb_in_priority[priority];
    }
    else
    {
        nya_sys_ctx.next_task = &nya_sys_ctx.tcb[index];
    }
}

/* ------------------------------------------------------------------------------ */
/* Global Declarations */
/* ------------------------------------------------------------------------------ */

void nya_task_switch(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    nya_u8_t highest_priority = nya_sys_ctx.ready_to_index_lookup[nya_sys_ctx.priority_group_cluster_ready];

    highest_priority = nya_sys_ctx.ready_to_index_lookup[nya_sys_ctx.priority_group_ready[highest_priority]] +
                       (highest_priority * 8);

    if (nya_sys_ctx.first_tcb_in_priority[highest_priority] != nya_sys_ctx.curr_task)
    {
        nya_sys_ctx.next_task = nya_sys_ctx.first_tcb_in_priority[highest_priority];

        NYA_CTX_SWITCH();
    }

    NYA_EXIT_CRITICAL();
}

/* ------------------------------------------------------------------------------ */
/* API Declarations */
/* ------------------------------------------------------------------------------ */

void nya_sys_init()
{
    nya_u32_t index = 0;

#define NYA_TASK(_priority, _stack_size) \
    _init_tcb(index++,                   \
              _priority,                 \
              _stack_size);
    NYA_TASK_DEFINITIONS
#undef NYA_TASK
}
