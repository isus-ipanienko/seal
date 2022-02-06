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

nya_sys_ctx_t os_ctx =
{
    .resolve_prio_lkp =
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
    .prio_indx_lkp    =
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
    .prio_mask_lkp    =
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

/**
 * @brief   Pops a priority queue.
 * @note    Doesn't support priority boosting, yet.
 *          TODO: add temp priority field to tcbs
 * @note    Always call this from within a critical section.
 */
static void _pop_priority(nya_u8_t priority);

/**
 * @brief   Pushes a task to a priority queue.
 * @note    Doesn't support priority boosting, yet.
 *          TODO: add temp priority field to tcbs
 * @note    Always call this from within a critical section.
 */
static void _push_priority(nya_size_t index,
                           nya_u8_t priority);

/**
 * @brief   Not fully implemented.
 *
 * @param index
 * @param priority
 * @param stack_size
 */
static void _init_tcb(nya_size_t index,
                      nya_u8_t priority,
                      nya_stack_t stack_size);

/* ------------------------------------------------------------------------------ */
/* Private Declarations */
/* ------------------------------------------------------------------------------ */

static void _pop_priority(nya_u8_t priority)
{
    /* if (os_ctx.prioq[priority].count == 0) nya_panic(); ? */

    if (--os_ctx.prioq[priority].count == 0)
    {
        os_ctx.prio_grp_cluster_rdy &= ~os_ctx.prio_mask_lkp[priority];
        os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[priority]] &= ~os_ctx.prio_mask_lkp[priority];
        os_ctx.prioq[priority].first = NYA_NULL;
        os_ctx.prioq[priority].last = NYA_NULL;
    }
    else
    {
        os_ctx.prioq[priority].first = os_ctx.prioq[priority].first->next_in_prio;
    }
}

static void _push_priority(nya_size_t index,
                           nya_u8_t priority)
{
    if (os_ctx.prioq[priority].count == 0)
    {
        os_ctx.tcb[index].prev_in_prio = NYA_NULL;
        os_ctx.tcb[index].next_in_prio = NYA_NULL;
        os_ctx.prioq[priority].first = &os_ctx.tcb[index];
        os_ctx.prioq[priority].last = &os_ctx.tcb[index];
    }
    else
    {
        os_ctx.tcb[index].prev_in_prio = os_ctx.prioq[priority].last;
        os_ctx.tcb[index].next_in_prio = NYA_NULL;
        os_ctx.prioq[priority].last->next_in_prio = &os_ctx.tcb[index];
        os_ctx.prioq[priority].last = &os_ctx.tcb[index];
    }

    os_ctx.prioq[priority].count++;
    os_ctx.prio_grp_cluster_rdy |= os_ctx.prio_mask_lkp[priority];
    os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[priority]] |= os_ctx.prio_mask_lkp[priority];
}

static void _init_tcb(nya_size_t index,
                      nya_u8_t priority,
                      nya_stack_t stack_size)
{
    os_ctx.tcb[index].priority = priority;
#if NYA_CFG_ENABLE_STATS
    nya_sys_ctx.tcb[index].stack_size = stack_size;
#endif /* if NYA_CFG_ENABLE_STATS */

    _push_priority(index,
                   priority);
}

/* ------------------------------------------------------------------------------ */
/* Global Declarations */
/* ------------------------------------------------------------------------------ */

void nya_task_switch(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (os_ctx.isr_nesting_cnt == 0)
    {
        nya_u8_t highest_priority = os_ctx.resolve_prio_lkp[os_ctx.prio_grp_cluster_rdy];

        highest_priority = os_ctx.resolve_prio_lkp[os_ctx.prio_grp_rdy[highest_priority]] +
                           (highest_priority * 8);

        if (os_ctx.prioq[highest_priority].first != os_ctx.curr_task)
        {
            os_ctx.next_task = os_ctx.prioq[highest_priority].first;

            NYA_CTX_SWITCH();
        }
    }

    NYA_EXIT_CRITICAL();
}

void nya_inc_systick(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    for (nya_size_t index = 0; index < NYA_CFG_TASK_CNT; index++)
    {
        if (os_ctx.tcb[index].delay)
        {
            if (--os_ctx.tcb[index].delay == 0)
            {
                _push_priority(index,
                               os_ctx.tcb[index].priority);
            }
        }
    }

    NYA_EXIT_CRITICAL();
}

/* ------------------------------------------------------------------------------ */
/* API Declarations */
/* ------------------------------------------------------------------------------ */

void nya_enter_isr(void)
{
    /*TODO: if (nya_sys_ctx.isr_nesting_cnt == 255) panic(); ? */
    os_ctx.isr_nesting_cnt++;
}

void nya_exit_isr(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    /*TODO: if (nya_sys_ctx.isr_nesting_cnt == 0) panic(); ? */
    os_ctx.isr_nesting_cnt--;

    if (os_ctx.isr_nesting_cnt == 0)
    {
        nya_u8_t highest_priority = os_ctx.resolve_prio_lkp[os_ctx.prio_grp_cluster_rdy];

        highest_priority = os_ctx.resolve_prio_lkp[os_ctx.prio_grp_rdy[highest_priority]] +
                           (highest_priority * 8);

        if (os_ctx.prioq[highest_priority].first != os_ctx.curr_task)
        {
            os_ctx.next_task = os_ctx.prioq[highest_priority].first;

            NYA_CTX_SWITCH_FROM_ISR();
        }
    }

    NYA_EXIT_CRITICAL();
}

void nya_sleep(nya_size_t ticks)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    os_ctx.curr_task->delay = ticks;
    _pop_priority(os_ctx.curr_task->priority);

    NYA_EXIT_CRITICAL();

    nya_task_switch();
}

void nya_sys_init()
{
    nya_size_t index = 0;

#define NYA_TASK(_priority, _stack_size) \
    _init_tcb(index++,                   \
              _priority,                 \
              _stack_size);
    NYA_TASK_DEFINITIONS
#undef NYA_TASK
}
