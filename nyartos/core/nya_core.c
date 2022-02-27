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
/* System Variables */
/* ------------------------------------------------------------------------------ */

nya_tcb_t *volatile nya_curr_task = NYA_NULL;
nya_tcb_t *volatile nya_next_task = NYA_NULL;

/* *INDENT-OFF* */
/* Stack Declarations */
#define NYA_TASK(_id,               \
                 _priority,         \
                 _stack_size,       \
                 _entry_func,       \
                 _entry_func_param) \
    static nya_stack_t nya_stack_##_id[NYA_PORT_BYTES_TO_SECTORS(_stack_size)];
    NYA_TASK_DEFINITIONS
#undef NYA_TASK

/* *INDENT-ON* */

nya_os_ctx_t os_ctx =
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
 * @brief   Initializes a single task control block.
 * @param id
 * @param base_prio
 * @param stack_size
 */
static void _init_tcb(nya_size_t id,
                      nya_u8_t base_prio,
                      nya_stack_t stack_size,
                      nya_stack_t *stack_base,
                      nya_task_func_t entry_func,
                      void *entry_func_param);

/**
 * @brief Sets @c nya_next_task to point at the first ready task with the highest priority.
 *
 * @return NYA_TRUE - a context switch is needed; NYA_FALSE - no context switch is needed
 */
nya_bool_t _set_next_task(void);

/* ------------------------------------------------------------------------------ */
/* Private Declarations */
/* ------------------------------------------------------------------------------ */

static void _init_tcb(nya_size_t id,
                      nya_u8_t base_prio,
                      nya_stack_t stack_size,
                      nya_stack_t *stack_base,
                      nya_task_func_t entry_func,
                      void *entry_func_param)
{
    os_ctx.tcb_l[id].tid = id;
    os_ctx.tcb_l[id].base_prio = base_prio;
    os_ctx.tcb_l[id].curr_prio = base_prio;
    os_ctx.tcb_l[id].stack_ptr = nya_port_init_stack(entry_func,
                                                     stack_base,
                                                     stack_size,
                                                     entry_func_param);
#if NYA_CFG_ENABLE_STATS
    os_ctx.tcb_l[id].stack_size = stack_size;
    os_ctx.tcb_l[id].stack_end = &stack_base[stack_size - 1];
#endif /* if NYA_CFG_ENABLE_STATS */

    nya_priority_push_last(&os_ctx.tcb_l[id]);
    os_ctx.tcb_l[id].state = NYA_TASK_READY;
}

nya_bool_t _set_next_task(void)
{
    if (os_ctx.isr_nesting_cnt == 0)
    {
        nya_u8_t highest_priority = os_ctx.resolve_prio_lkp[os_ctx.prio_grp_cluster_rdy];

        highest_priority = os_ctx.resolve_prio_lkp[os_ctx.prio_grp_rdy[highest_priority]] +
                           (highest_priority * 8);

        if (os_ctx.prioq_l[highest_priority].first != nya_next_task)
        {
            nya_next_task = os_ctx.prioq_l[highest_priority].first;

            return NYA_TRUE;
        }
    }

    return NYA_FALSE;
}

/* ------------------------------------------------------------------------------ */
/* Global Declarations */
/* ------------------------------------------------------------------------------ */

void nya_core_panic(void)
{
    NYA_DISABLE_INTERRUPTS();
    nya_panic_hook();

    while (1)
    {}
}

void nya_core_task_exit(void)
{
    nya_task_exit_hook();
    nya_core_panic();
}

void nya_core_schedule(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (_set_next_task())
    {
        NYA_CTX_SWITCH();
    }

    NYA_EXIT_CRITICAL();
}

void nya_core_systick(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    /* TODO: create a waiting list with delayed tasks */
    for (nya_size_t id = 0; id < NYA_TASK_ID_CNT; id++)
    {
        if (os_ctx.tcb_l[id].delay)
        {
            if (--os_ctx.tcb_l[id].delay == 0)
            {
                if (os_ctx.tcb_l[id].state == NYA_TASK_WAITING_FOR_EVENT)
                {
                    nya_mutex_timeout(&os_ctx.tcb_l[id]);
                }
                else if (os_ctx.tcb_l[id].state != NYA_TASK_ASLEEP)
                {
                    nya_core_panic();
                }

                nya_priority_push_last(&os_ctx.tcb_l[id]);
                os_ctx.tcb_l[id].state = NYA_TASK_READY;
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
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (os_ctx.isr_nesting_cnt == 255)
    {
        nya_core_panic();
    }

    os_ctx.isr_nesting_cnt++;
    NYA_EXIT_CRITICAL();
}

void nya_exit_isr(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (os_ctx.isr_nesting_cnt == 0)
    {
        nya_core_panic();
    }

    os_ctx.isr_nesting_cnt--;

    if (_set_next_task())
    {
        NYA_CTX_SWITCH_FROM_ISR();
    }

    NYA_EXIT_CRITICAL();
}

void nya_sleep(nya_size_t ticks)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    nya_curr_task->state = NYA_TASK_ASLEEP;
    nya_curr_task->delay = ticks;
    nya_priority_pop(nya_curr_task);

    NYA_EXIT_CRITICAL();

    nya_core_schedule();
}

void nya_init()
{
#define NYA_TASK(_id,                                 \
                 _priority,                           \
                 _stack_size,                         \
                 _entry_func,                         \
                 _entry_func_param)                   \
    _init_tcb(_id,                                    \
              _priority,                              \
              NYA_PORT_BYTES_TO_SECTORS(_stack_size), \
              nya_stack_##_id,                        \
              _entry_func,                            \
              _entry_func_param);
    NYA_TASK_DEFINITIONS
#undef NYA_TASK

#define NYA_MUTEX(_id) \
    nya_mutex_init(_id);

    NYA_MUTEX_DEFINITIONS
#undef NYA_MUTEX

    if (_set_next_task())
    {
        nya_port_startup();
    }

    /* nya_port_startup(); should never return */
    nya_core_panic();
}
