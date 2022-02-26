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
/* Stacks */
/* ------------------------------------------------------------------------------ */

/* *INDENT-OFF* */

#define NYA_TASK(_id,               \
                 _priority,         \
                 _stack_size,       \
                 _entry_func,       \
                 _entry_func_param) \
    static nya_stack_t nya_stack_##_id[NYA_PORT_BYTES_TO_SECTORS(_stack_size)];
    NYA_TASK_DEFINITIONS
#undef NYA_TASK

/* *INDENT-ON* */

/* ------------------------------------------------------------------------------ */
/* System Context */
/* ------------------------------------------------------------------------------ */

nya_tcb_t *volatile nya_curr_task = NYA_NULL;
nya_tcb_t *volatile nya_next_task = NYA_NULL;
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

/* ------------------------------------------------------------------------------ */
/* API Declarations */
/* ------------------------------------------------------------------------------ */

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

    nya_scheduler_set_next_task();

    nya_port_startup();

    /* nya_port_startup(); should never return */
    nya_core_panic();
}
