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

void nya_core_sort_event_waiting_lists(nya_tcb_t *task,
                                       nya_u8_t new_prio)
{
    nya_tcb_t *super_holder = NYA_NULL;
    nya_tcb_t *waiting_holder = task;

    do
    {
        super_holder = waiting_holder->wait_event->holder;

        if (waiting_holder->wait_event->waiting_l == waiting_holder)
        {
            if (super_holder->state == NYA_TASK_READY)
            {
                nya_priority_remove(super_holder);
            }

            super_holder->curr_prio = new_prio;
            nya_priority_push_first(super_holder);
        }
        else
        {
            if (waiting_holder->prev_in_eventq != NYA_NULL)
            {
                waiting_holder->prev_in_eventq->next_in_eventq = waiting_holder->next_in_eventq;
            }

            if (waiting_holder->next_in_eventq != NYA_NULL)
            {
                waiting_holder->next_in_eventq->prev_in_eventq = waiting_holder->prev_in_eventq;
            }

            nya_tcb_t *place_in_waiting_l = waiting_holder->wait_event->waiting_l;

            while ((place_in_waiting_l->next_in_eventq != NYA_NULL) && (place_in_waiting_l->next_in_eventq->curr_prio <=
                                                                        waiting_holder->curr_prio))
            {
                place_in_waiting_l = place_in_waiting_l->next_in_eventq;
            }

            waiting_holder->prev_in_eventq = place_in_waiting_l;
            waiting_holder->next_in_eventq = place_in_waiting_l->next_in_eventq;

            if (place_in_waiting_l->next_in_eventq != NYA_NULL)
            {
                place_in_waiting_l->next_in_eventq->prev_in_eventq = waiting_holder;
                place_in_waiting_l->next_in_eventq = waiting_holder;
            }
        }

        waiting_holder = super_holder;
    } while (super_holder->state == NYA_TASK_WAITING_FOR_EVENT);
}
