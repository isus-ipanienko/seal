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
            nya_priority_remove(holder);
            holder->curr_prio = new_prio;
            nya_priority_push_first(holder);
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
    if (event->waiting_l == NYA_NULL)
    {
        return NYA_NULL;
    }

    nya_tcb_t *high_prio_task = event->waiting_l;
    nya_tcb_t *next_task = event->waiting_l->next_in_ready_q;

    while (next_task != NYA_NULL)
    {
        if (next_task->curr_prio <= high_prio_task->curr_prio)
        {
            high_prio_task = next_task;
        }

        next_task = next_task->next_in_ready_q;
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
    if (os_ctx.event_l[id].type == NYA_EVENT_BOTTOM)
    {
        os_ctx.event_l[id].type = type;

        switch (type)
        {
        case NYA_EVENT_SEMAPHORE:
            os_ctx.event_l[id].count = count;
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

    if ((task->wait_event->type != NYA_EVENT_MUTEX) && (task->wait_event->type != NYA_EVENT_SEMAPHORE))
    {
        NYA_EXIT_CRITICAL();
        nya_core_panic();
    }

    task->wait_return = NYA_WAIT_RET_TIMEOUT;

    if (task->wait_event->waiting_l == task)
    {
        task->wait_event->waiting_l = task->next_in_ready_q;
    }

    if (task->prev_in_ready_q != NYA_NULL)
    {
        task->prev_in_ready_q->next_in_ready_q = task->next_in_ready_q;
    }

    if (task->next_in_ready_q != NYA_NULL)
    {
        task->next_in_ready_q->prev_in_ready_q = task->prev_in_ready_q;
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

    if (os_ctx.event_l[id].type != NYA_EVENT_MUTEX)
    {
        NYA_EXIT_CRITICAL();

        return NYA_WRONG_EVENT;
    }

    nya_curr_task->wait_return = NYA_WAIT_RET_OK;
    nya_tcb_t *holder = os_ctx.event_l[id].holder;

    if (holder != NYA_NULL)
    {
        nya_curr_task->state = NYA_TASK_WAITING_FOR_EVENT;
        nya_curr_task->delay = timeout;
        nya_curr_task->wait_event = &os_ctx.event_l[id];
        nya_priority_pop(nya_curr_task);

        if (holder->curr_prio > nya_curr_task->curr_prio)
        {
            _mutex_update_priorities(holder,
                                     nya_curr_task->curr_prio);
        }

        nya_curr_task->prev_in_ready_q = NYA_NULL;
        nya_curr_task->next_in_ready_q = os_ctx.event_l[id].waiting_l;
        os_ctx.event_l[id].waiting_l = nya_curr_task;

        NYA_EXIT_CRITICAL();

        nya_core_schedule();
    }
    else
    {
        os_ctx.event_l[id].holder = nya_curr_task;
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

    if (os_ctx.event_l[id].type != NYA_EVENT_MUTEX)
    {
        NYA_EXIT_CRITICAL();

        return NYA_WRONG_EVENT;
    }

    if (nya_curr_task->curr_prio != nya_curr_task->base_prio)
    {
        nya_priority_pop(nya_curr_task);
        nya_curr_task->curr_prio = nya_curr_task->base_prio;
        nya_priority_push_first(nya_curr_task);
    }

    nya_tcb_t *high_prio_task = _get_event_high_prio_task(&os_ctx.event_l[id]);

    if (high_prio_task != NYA_NULL)
    {
        os_ctx.event_l[id].holder = high_prio_task;

        if (os_ctx.event_l[id].waiting_l == os_ctx.event_l[id].holder)
        {
            os_ctx.event_l[id].waiting_l = os_ctx.event_l[id].waiting_l->next_in_ready_q;
        }

        if (os_ctx.event_l[id].holder->prev_in_ready_q != NYA_NULL)
        {
            os_ctx.event_l[id].holder->prev_in_ready_q->next_in_ready_q = os_ctx.event_l[id].holder->next_in_ready_q;
        }

        if (os_ctx.event_l[id].holder->next_in_ready_q != NYA_NULL)
        {
            os_ctx.event_l[id].holder->next_in_ready_q->prev_in_ready_q = os_ctx.event_l[id].holder->prev_in_ready_q;
        }

        os_ctx.event_l[id].holder->state = NYA_TASK_READY;
        os_ctx.event_l[id].holder->delay = 0;
        nya_priority_push_last(os_ctx.event_l[id].holder);

        NYA_EXIT_CRITICAL();

        nya_core_schedule();
    }
    else
    {
        os_ctx.event_l[id].holder = NYA_NULL;
        NYA_EXIT_CRITICAL();
    }

    return NYA_OK;
}

nya_error_t nya_semaphore_take(nya_event_id_t id,
                               nya_size_t timeout)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (os_ctx.event_l[id].type != NYA_EVENT_SEMAPHORE)
    {
        NYA_EXIT_CRITICAL();

        return NYA_WRONG_EVENT;
    }

    nya_curr_task->wait_return = NYA_WAIT_RET_OK;

    if (os_ctx.event_l[id].count == 0)
    {
        nya_curr_task->state = NYA_TASK_WAITING_FOR_EVENT;
        nya_curr_task->delay = timeout;
        nya_curr_task->wait_event = &os_ctx.event_l[id];
        nya_priority_pop(nya_curr_task);

        if (os_ctx.event_l[id].waiting_l == NYA_NULL)
        {
            os_ctx.event_l[id].waiting_l = nya_curr_task;
            nya_curr_task->prev_in_ready_q = NYA_NULL;
            nya_curr_task->next_in_ready_q = NYA_NULL;
        }
        else
        {
            nya_tcb_t *last = os_ctx.event_l[id].waiting_l;

            while (last->next_in_ready_q != NYA_NULL)
            {
                last = last->next_in_ready_q;
            }

            last->next_in_ready_q = nya_curr_task;
            nya_curr_task->prev_in_ready_q = last;
            nya_curr_task->next_in_ready_q = NYA_NULL;
        }

        NYA_EXIT_CRITICAL();

        nya_core_schedule();
    }
    else
    {
        os_ctx.event_l[id].count--;
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

    if (os_ctx.event_l[id].type != NYA_EVENT_SEMAPHORE)
    {
        NYA_EXIT_CRITICAL();

        return NYA_WRONG_EVENT;
    }

    os_ctx.event_l[id].count++;

    if (os_ctx.event_l[id].waiting_l != NYA_NULL)
    {
        os_ctx.event_l[id].waiting_l->state = NYA_TASK_READY;
        os_ctx.event_l[id].waiting_l->delay = 0;
        os_ctx.event_l[id].waiting_l = os_ctx.event_l[id].waiting_l->next_in_ready_q;
        nya_priority_push_last(os_ctx.event_l[id].waiting_l);

        NYA_EXIT_CRITICAL();

        nya_core_schedule();
    }
    else
    {
        NYA_EXIT_CRITICAL();
    }

    return NYA_OK;
}
