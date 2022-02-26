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
/* Global Declarations */
/* ------------------------------------------------------------------------------ */

void nya_mutex_init(nya_event_id_t id)
{
    if (os_ctx.event_l[id].type == NYA_EVENT_BOTTOM)
    {
        os_ctx.event_l[id].type = NYA_EVENT_MUTEX;
    }
}

void nya_mutex_timeout(nya_tcb_t *task)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (task->wait_event->type != NYA_EVENT_MUTEX)
    {
        NYA_EXIT_CRITICAL();
        nya_core_panic();
    }

    task->wait_return = NYA_WAIT_RET_TIMEOUT;

    if (task->wait_event->waiting_l == task)
    {
        task->wait_event->waiting_l = task->next_in_eventq;
    }

    if (task->prev_in_eventq != NYA_NULL)
    {
        task->prev_in_eventq->next_in_eventq = task->next_in_eventq;
    }

    if (task->next_in_eventq != NYA_NULL)
    {
        task->next_in_eventq->prev_in_eventq = task->prev_in_eventq;
    }

    nya_tcb_t *holder = task->wait_event->holder;
    nya_u8_t holder_next_prio = holder->base_prio;

    if ((task->wait_event->waiting_l != NYA_NULL) && (task->wait_event->waiting_l->curr_prio < holder_next_prio))
    {
        holder_next_prio = task->wait_event->waiting_l->curr_prio;
    }

    if (holder->curr_prio != holder_next_prio)
    {
        if (holder->state == NYA_TASK_READY)
        {
            nya_priority_remove(holder);
        }
        else if (holder->state == NYA_TASK_WAITING_FOR_EVENT)
        {
            nya_core_sort_event_waiting_lists(holder,
                                              holder_next_prio);
        }

        holder->curr_prio = holder_next_prio;
        nya_priority_push_first(holder);
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
            if (holder->state == NYA_TASK_READY)
            {
                nya_priority_remove(holder);
            }
            else if (holder->state == NYA_TASK_WAITING_FOR_EVENT)
            {
                nya_core_sort_event_waiting_lists(holder,
                                                  nya_next_task->curr_prio);
            }

            holder->curr_prio = nya_curr_task->curr_prio;
            nya_priority_push_first(holder);
            nya_curr_task->prev_in_eventq = NYA_NULL;
            nya_curr_task->next_in_eventq = os_ctx.event_l[id].waiting_l;
            os_ctx.event_l[id].waiting_l = nya_curr_task;
        }
        else
        {
            if (os_ctx.event_l[id].waiting_l == NYA_NULL)
            {
                nya_curr_task->prev_in_eventq = NYA_NULL;
                nya_curr_task->next_in_eventq = NYA_NULL;
                os_ctx.event_l[id].waiting_l = nya_curr_task;
            }
            else
            {
                nya_tcb_t *place_in_waiting_l = os_ctx.event_l[id].waiting_l;

                while ((place_in_waiting_l->next_in_eventq != NYA_NULL) &&
                       (place_in_waiting_l->next_in_eventq->curr_prio <= nya_curr_task->curr_prio))
                {
                    place_in_waiting_l = place_in_waiting_l->next_in_eventq;
                }

                nya_curr_task->prev_in_eventq = place_in_waiting_l;
                nya_curr_task->next_in_eventq = place_in_waiting_l->next_in_eventq;

                if (place_in_waiting_l->next_in_eventq != NYA_NULL)
                {
                    place_in_waiting_l->next_in_eventq->prev_in_eventq = nya_curr_task;
                    place_in_waiting_l->next_in_eventq = nya_curr_task;
                }
            }
        }

        NYA_EXIT_CRITICAL();

        nya_scheduler_switch();
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

    if (os_ctx.event_l[id].waiting_l != NYA_NULL)
    {
        os_ctx.event_l[id].holder = os_ctx.event_l[id].waiting_l;
        os_ctx.event_l[id].waiting_l = os_ctx.event_l[id].waiting_l->next_in_eventq;
        os_ctx.event_l[id].holder->state = NYA_TASK_READY;
        os_ctx.event_l[id].holder->delay = 0;
        nya_priority_push_last(os_ctx.event_l[id].holder);

        NYA_EXIT_CRITICAL();

        nya_scheduler_switch();
    }
    else
    {
        os_ctx.event_l[id].holder = NYA_NULL;
        NYA_EXIT_CRITICAL();
    }

    return NYA_OK;
}
