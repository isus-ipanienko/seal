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
/* Private Prototypes */
/* ------------------------------------------------------------------------------ */

/**
 * @brief This function updates priorities of a mutex holder and each mutex it's waiting for.
 * @param [in] holder - pointer to the holder
 * @param [in] new_prio - new priority
 */
static void _mutex_update_priorities(nya_tcb_t *holder,
                                     nya_u8_t new_prio);

/**
 * @brief   Gets the last highest priority task in a mutex queue. Tasks are always inserted at the beginning of a queue,
 *          so the last task with a given priority is waiting for the longest time.
 * @param   [in] event - pointer to a mutex struct
 * @return  nya_tcb_t* - pointer to the task
 */
static nya_tcb_t* _get_event_high_prio_task(const nya_event_t *const event);

/* ------------------------------------------------------------------------------ */
/* Private Declarations */
/* ------------------------------------------------------------------------------ */

static void _mutex_update_priorities(nya_tcb_t *holder,
                                     nya_u8_t new_prio)
{
    while (holder->curr_prio != new_prio)
    {
        if (holder->state == NYA_TASK_READY)
        {
            nya_queue_remove(holder, &os_ctx.priorities[holder->curr_prio]);
            if (os_ctx.priorities[holder->curr_prio].first == NYA_NULL)
            {
                NYA_PRIORITY_UNREADY(holder->curr_prio);
            };
            holder->curr_prio = new_prio;
            nya_queue_push(holder, &os_ctx.priorities[holder->curr_prio]);
            NYA_PRIORITY_READY(holder->curr_prio);
        }
        else if (holder->state == NYA_TASK_WAITING_FOR_EVENT)
        {
            holder->curr_prio = new_prio;
            new_prio = _get_event_high_prio_task(holder->wait_event)->curr_prio;
            holder = holder->wait_event->holder;
        }
    }
}

static nya_tcb_t* _get_event_high_prio_task(const nya_event_t *const event)
{
    if (event->waiting_queue == NYA_NULL)
    {
        return NYA_NULL;
    }

    nya_tcb_t *high_prio_task = event->waiting_queue;
    nya_tcb_t *next_task = event->waiting_queue->next;

    while (next_task != NYA_NULL)
    {
        if (next_task->curr_prio <= high_prio_task->curr_prio)
        {
            high_prio_task = next_task;
        }

        next_task = next_task->next;
    }

    return high_prio_task;
}

/* ------------------------------------------------------------------------------ */
/* Global Declarations */
/* ------------------------------------------------------------------------------ */

void nya_event_init(nya_event_id_t id,
                    nya_event_type_t type,
                    nya_u32_t count)
{
    if (os_ctx.events[id].type == NYA_EVENT_UNINITIALIZED)
    {
        os_ctx.events[id].type = type;

        switch (type)
        {
        case NYA_EVENT_SEMAPHORE:
            os_ctx.events[id].count = count;
            break;
        default:
            break;
        }
    }
    else
    {
        nya_core_panic();
    }
}

void nya_event_timeout(nya_tcb_t *task)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if ((task->wait_event->type == NYA_EVENT_UNINITIALIZED) || (task->wait_event->type >= NYA_EVENT_TOP))
    {
        NYA_EXIT_CRITICAL();
        nya_core_panic();
    }

    task->wait_return = NYA_WAIT_RET_TIMEOUT;

    if (task->wait_event->waiting_queue == task)
    {
        task->wait_event->waiting_queue = task->next;
    }

    if (task->prev != NYA_NULL)
    {
        task->prev->next = task->next;
    }

    if (task->next != NYA_NULL)
    {
        task->next->prev = task->prev;
    }

    if (task->wait_event->type == NYA_EVENT_MUTEX)
    {
        nya_tcb_t *high_prio_task = _get_event_high_prio_task(task->wait_event);

        if (high_prio_task != NYA_NULL)
        {
            _mutex_update_priorities(task->wait_event->holder,
                                     high_prio_task->curr_prio);
        }
    }

    NYA_EXIT_CRITICAL();
}

/* ------------------------------------------------------------------------------ */
/* API Declarations */
/* ------------------------------------------------------------------------------ */

nya_error_t nya_mutex_take(nya_event_id_t id,
                           nya_size_t timeout)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (os_ctx.events[id].type != NYA_EVENT_MUTEX)
    {
        NYA_EXIT_CRITICAL();

        return NYA_WRONG_EVENT;
    }

    nya_curr_task->wait_return = NYA_WAIT_RET_OK;
    nya_tcb_t *holder = os_ctx.events[id].holder;

    if (holder != NYA_NULL)
    {
        nya_curr_task->state = NYA_TASK_WAITING_FOR_EVENT;
        nya_curr_task->delay = timeout;
        nya_curr_task->wait_event = &os_ctx.events[id];
        nya_queue_pop(&os_ctx.priorities[nya_curr_task->curr_prio]);
        if (os_ctx.priorities[nya_curr_task->curr_prio].first == NYA_NULL)
        {
            NYA_PRIORITY_UNREADY(nya_curr_task->curr_prio);
        };

        if (holder->curr_prio < nya_curr_task->curr_prio)
        {
            _mutex_update_priorities(holder,
                                     nya_curr_task->curr_prio);
        }

        nya_curr_task->prev = NYA_NULL;
        nya_curr_task->next = os_ctx.events[id].waiting_queue;
        os_ctx.events[id].waiting_queue = nya_curr_task;

        NYA_EXIT_CRITICAL();

        nya_core_schedule();
    }
    else
    {
        os_ctx.events[id].holder = nya_curr_task;
        NYA_EXIT_CRITICAL();
    }

    switch (nya_curr_task->wait_return)
    {
    case NYA_WAIT_RET_OK:
        return NYA_OK;

    case NYA_WAIT_RET_TIMEOUT:
        return NYA_TIMEOUT;

    default:
        return NYA_ERROR;
    }
}

nya_error_t nya_mutex_give(nya_event_id_t id)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (os_ctx.events[id].type != NYA_EVENT_MUTEX)
    {
        NYA_EXIT_CRITICAL();

        return NYA_WRONG_EVENT;
    }

    if (nya_curr_task->curr_prio != nya_curr_task->base_prio)
    {
        nya_queue_remove(nya_curr_task, &os_ctx.priorities[nya_curr_task->curr_prio]);
        if (os_ctx.priorities[nya_curr_task->curr_prio].first == NYA_NULL)
        {
            NYA_PRIORITY_UNREADY(nya_curr_task->curr_prio);
        };
        nya_curr_task->curr_prio = nya_curr_task->base_prio;
        nya_queue_push(nya_curr_task, &os_ctx.priorities[nya_curr_task->curr_prio]);
        NYA_PRIORITY_READY(nya_curr_task->curr_prio);
    }

    nya_tcb_t *high_prio_task = _get_event_high_prio_task(&os_ctx.events[id]);

    if (high_prio_task != NYA_NULL)
    {
        os_ctx.events[id].holder = high_prio_task;

        if (os_ctx.events[id].waiting_queue == os_ctx.events[id].holder)
        {
            os_ctx.events[id].waiting_queue = os_ctx.events[id].waiting_queue->next;
        }

        if (os_ctx.events[id].holder->prev != NYA_NULL)
        {
            os_ctx.events[id].holder->prev->next = os_ctx.events[id].holder->next;
        }

        if (os_ctx.events[id].holder->next != NYA_NULL)
        {
            os_ctx.events[id].holder->next->prev = os_ctx.events[id].holder->prev;
        }

        os_ctx.events[id].holder->state = NYA_TASK_READY;
        os_ctx.events[id].holder->delay = 0;
        nya_queue_push(os_ctx.events[id].holder, &os_ctx.priorities[os_ctx.events[id].holder->curr_prio]);
        NYA_PRIORITY_READY(os_ctx.events[id].holder->curr_prio);

        NYA_EXIT_CRITICAL();

        nya_core_schedule();
    }
    else
    {
        os_ctx.events[id].holder = NYA_NULL;
        NYA_EXIT_CRITICAL();
    }

    return NYA_OK;
}

nya_error_t nya_semaphore_take(nya_event_id_t id,
                               nya_size_t timeout)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (os_ctx.events[id].type != NYA_EVENT_SEMAPHORE)
    {
        NYA_EXIT_CRITICAL();

        return NYA_WRONG_EVENT;
    }

    nya_curr_task->wait_return = NYA_WAIT_RET_OK;

    if (os_ctx.events[id].count == 0)
    {
        nya_curr_task->state = NYA_TASK_WAITING_FOR_EVENT;
        nya_curr_task->delay = timeout;
        nya_curr_task->wait_event = &os_ctx.events[id];
        nya_queue_pop(&os_ctx.priorities[nya_curr_task->curr_prio]);
        if (os_ctx.priorities[nya_curr_task->curr_prio].first == NYA_NULL)
        {
            NYA_PRIORITY_UNREADY(nya_curr_task->curr_prio);
        };

        if (os_ctx.events[id].waiting_queue == NYA_NULL)
        {
            os_ctx.events[id].waiting_queue = nya_curr_task;
            nya_curr_task->prev = NYA_NULL;
            nya_curr_task->next = NYA_NULL;
        }
        else
        {
            nya_tcb_t *last = os_ctx.events[id].waiting_queue;

            while (last->next != NYA_NULL)
            {
                last = last->next;
            }

            last->next = nya_curr_task;
            nya_curr_task->prev = last;
            nya_curr_task->next = NYA_NULL;
        }

        NYA_EXIT_CRITICAL();

        nya_core_schedule();
    }
    else
    {
        os_ctx.events[id].count--;
        NYA_EXIT_CRITICAL();
    }

    switch (nya_curr_task->wait_return)
    {
    case NYA_WAIT_RET_OK:
        return NYA_OK;

    case NYA_WAIT_RET_TIMEOUT:
        return NYA_TIMEOUT;

    default:
        return NYA_ERROR;
    }
}

nya_error_t nya_semaphore_give(nya_event_id_t id)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (os_ctx.events[id].type != NYA_EVENT_SEMAPHORE)
    {
        NYA_EXIT_CRITICAL();

        return NYA_WRONG_EVENT;
    }

    os_ctx.events[id].count++;

    if (os_ctx.events[id].waiting_queue != NYA_NULL)
    {
        os_ctx.events[id].waiting_queue->state = NYA_TASK_READY;
        os_ctx.events[id].waiting_queue->delay = 0;
        os_ctx.events[id].waiting_queue = os_ctx.events[id].waiting_queue->next;
        nya_queue_push(os_ctx.events[id].waiting_queue, &os_ctx.priorities[os_ctx.events[id].waiting_queue->curr_prio]);
        NYA_PRIORITY_READY(os_ctx.events[id].waiting_queue->curr_prio);

        NYA_EXIT_CRITICAL();

        nya_core_schedule();
    }
    else
    {
        NYA_EXIT_CRITICAL();
    }

    return NYA_OK;
}
