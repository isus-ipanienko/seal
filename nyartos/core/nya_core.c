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

nya_os_ctx_t os_ctx = {0};

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
    os_ctx.tcbs[id].tid = id;
    os_ctx.tcbs[id].base_prio = base_prio;
    os_ctx.tcbs[id].curr_prio = base_prio;
    os_ctx.tcbs[id].stack_ptr = nya_port_init_stack(entry_func,
                                                    stack_base,
                                                    stack_size,
                                                    entry_func_param);
#if NYA_CFG_ENABLE_STATS
    os_ctx.tcbs[id].stack_size = stack_size;
    os_ctx.tcbs[id].stack_end = &stack_base[stack_size - 1];
#endif /* if NYA_CFG_ENABLE_STATS */
    nya_queue_push(&os_ctx.tcbs[id], &os_ctx.priorities[base_prio]);
    NYA_PRIORITY_READY(base_prio);
    os_ctx.tcbs[id].state = NYA_TASK_READY;
}

nya_bool_t _set_next_task(void)
{
    if (os_ctx.isr_nesting_cnt == 0)
    {
        nya_u8_t highest_priority = NYA_GET_HIGHEST_PRIORITY(os_ctx.ready_priorities);

        if (os_ctx.priorities[highest_priority].first != nya_next_task)
        {
            nya_next_task = os_ctx.priorities[highest_priority].first;

            return NYA_TRUE;
        }
    }

    return NYA_FALSE;
}

/* ------------------------------------------------------------------------------ */
/* Internal System Declarations */
/* ------------------------------------------------------------------------------ */

void nya_panic(nya_error_t reason)
{
    NYA_DISABLE_INTERRUPTS();
    nya_panic_hook(reason);

    while (1)
    {}
}

void nya_task_exit(void)
{
    nya_task_exit_hook();
    nya_panic(NYA_TASK_EXITED);
}

void nya_schedule(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (_set_next_task())
    {
        NYA_CTX_SWITCH();
    }

    NYA_EXIT_CRITICAL();
}

void nya_systick(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    /* TODO: create a waiting list with delayed tasks */
    for (nya_size_t id = 0; id < NYA_TASK_ID_CNT; id++)
    {
        if (os_ctx.tcbs[id].delay)
        {
            if (--os_ctx.tcbs[id].delay == 0)
            {
                switch (os_ctx.tcbs[id].state)
                {
                case NYA_TASK_WAITING_FOR_EVENT:
                    nya_event_timeout(&os_ctx.tcbs[id]);
                    break;

                case NYA_TASK_ASLEEP:
                    break;

                default:
                    nya_panic(NYA_ERROR);
                }

                nya_queue_push(&os_ctx.tcbs[id], &os_ctx.priorities[os_ctx.tcbs[id].curr_prio]);
                NYA_PRIORITY_READY(os_ctx.tcbs[id].curr_prio);
                os_ctx.tcbs[id].state = NYA_TASK_READY;
            }
        }
    }

    NYA_EXIT_CRITICAL();
}

void nya_queue_push(nya_tcb_t *task, nya_queue_t *queue)
{
    NYA_ASSERT((task != NYA_NULL) && (queue != NYA_NULL), NYA_NULL_PARAM);

    if (queue->first == NYA_NULL)
    {
        task->prev = NYA_NULL;
        task->next = NYA_NULL;
        queue->first = task;
        queue->last = task;
    }
    else
    {
        task->prev = queue->last;
        task->next = NYA_NULL;
        queue->last->next = task;
        queue->last = task;
    }
}

void nya_queue_pop(nya_queue_t *queue)
{
    NYA_ASSERT((queue->first != NYA_NULL), NYA_NULL_PARAM);

    if (queue->first->next == NYA_NULL)
    {
        queue->first = NYA_NULL;
        queue->last = NYA_NULL;
    }
    else
    {
        queue->first = queue->first->next;
    }
}

void nya_queue_remove(nya_tcb_t *task, nya_queue_t *queue)
{
    if (queue->first == task)
    {
        queue->first = task->next;
    }

    if (queue->last == task)
    {
        queue->last = task->prev;
    }

    if (task->prev != NYA_NULL)
    {
        task->prev->next = task->next;
    }

    if (task->next != NYA_NULL)
    {
        task->next->prev = task->prev;
    }
}

void nya_update_priority(nya_tcb_t *task, nya_u8_t new_prio)
{
    if (task->curr_prio == new_prio)
    {
        return;
    }

    nya_queue_remove(task, &os_ctx.priorities[task->curr_prio]);
    NYA_PRIORITY_UNREADY(task->curr_prio);
    task->curr_prio = new_prio;
    nya_queue_push(task, &os_ctx.priorities[task->curr_prio]);
    NYA_PRIORITY_READY(task->curr_prio);
}

/* ------------------------------------------------------------------------------ */
/* API Declarations */
/* ------------------------------------------------------------------------------ */

void nya_enter_isr(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    NYA_ASSERT((os_ctx.isr_nesting_cnt != 255), NYA_ISR_OVERFLOW);

    os_ctx.isr_nesting_cnt++;
    NYA_EXIT_CRITICAL();
}

void nya_exit_isr(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    NYA_ASSERT((os_ctx.isr_nesting_cnt != 0), NYA_ISR_UNDERFLOW);

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
    nya_queue_pop(&os_ctx.priorities[nya_curr_task->curr_prio]);
    NYA_PRIORITY_UNREADY(nya_curr_task->curr_prio);
    NYA_EXIT_CRITICAL();
    nya_schedule();
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

#define NYA_MUTEX(_id)              \
    nya_event_init(_id,             \
                   NYA_EVENT_MUTEX, \
                   0);

    NYA_MUTEX_DEFINITIONS
#undef NYA_MUTEX

#define NYA_SEMAPHORE(_id, _count)      \
    nya_event_init(_id,                 \
                   NYA_EVENT_SEMAPHORE, \
                   _count);

    NYA_SEMAPHORE_DEFINITIONS
#undef NYA_SEMAPHORE

    if (_set_next_task())
    {
        nya_port_startup();
    }

    nya_panic(NYA_STARTUP_EXITED);
}
