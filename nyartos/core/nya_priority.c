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

void nya_priority_pop(nya_tcb_t *task)
{
    if (os_ctx.prioq_l[task->curr_prio].first == NYA_NULL)
    {
        nya_core_panic();
    }

    if (os_ctx.prioq_l[task->curr_prio].first->next_in_prioq == NYA_NULL)
    {
        os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[task->curr_prio]] &= ~os_ctx.prio_mask_lkp[task->curr_prio];

        if (os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[task->curr_prio]] == 0)
        {
            os_ctx.prio_grp_cluster_rdy &= ~os_ctx.prio_mask_lkp[os_ctx.prio_indx_lkp[task->curr_prio]];
        }

        os_ctx.prioq_l[task->curr_prio].first = NYA_NULL;
        os_ctx.prioq_l[task->curr_prio].last = NYA_NULL;
    }
    else
    {
        os_ctx.prioq_l[task->curr_prio].first = os_ctx.prioq_l[task->curr_prio].first->next_in_prioq;
    }
}

void nya_priority_remove(nya_tcb_t *task)
{
    if (os_ctx.prioq_l[task->curr_prio].first == task)
    {
        os_ctx.prioq_l[task->curr_prio].first = task->next_in_prioq;
    }

    if (os_ctx.prioq_l[task->curr_prio].last == task)
    {
        os_ctx.prioq_l[task->curr_prio].last = task->prev_in_prioq;
    }

    if (task->prev_in_prioq != NYA_NULL)
    {
        task->prev_in_prioq->next_in_prioq = task->next_in_prioq;
    }

    if (task->next_in_prioq != NYA_NULL)
    {
        task->next_in_prioq->prev_in_prioq = task->prev_in_prioq;
    }

    if (os_ctx.prioq_l[task->curr_prio].first == NYA_NULL)
    {
        os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[task->curr_prio]] &= ~os_ctx.prio_mask_lkp[task->curr_prio];

        if (os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[task->curr_prio]] == 0)
        {
            os_ctx.prio_grp_cluster_rdy &= ~os_ctx.prio_mask_lkp[os_ctx.prio_indx_lkp[task->curr_prio]];
        }
    }
}

void nya_priority_push_last(nya_tcb_t *task)
{
    if (os_ctx.prioq_l[task->curr_prio].first == NYA_NULL)
    {
        task->prev_in_prioq = NYA_NULL;
        task->next_in_prioq = NYA_NULL;
        os_ctx.prioq_l[task->curr_prio].first = task;
        os_ctx.prioq_l[task->curr_prio].last = task;
        os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[task->curr_prio]] |= os_ctx.prio_mask_lkp[task->curr_prio];
        os_ctx.prio_grp_cluster_rdy |= os_ctx.prio_mask_lkp[os_ctx.prio_indx_lkp[task->curr_prio]];
    }
    else
    {
        task->prev_in_prioq = os_ctx.prioq_l[task->curr_prio].last;
        task->next_in_prioq = NYA_NULL;
        os_ctx.prioq_l[task->curr_prio].last->next_in_prioq = task;
        os_ctx.prioq_l[task->curr_prio].last = task;
    }
}

void nya_priority_push_first(nya_tcb_t *task)
{
    if (os_ctx.prioq_l[task->curr_prio].first == NYA_NULL)
    {
        task->prev_in_prioq = NYA_NULL;
        task->next_in_prioq = NYA_NULL;
        os_ctx.prioq_l[task->curr_prio].first = task;
        os_ctx.prioq_l[task->curr_prio].last = task;
        os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[task->curr_prio]] |= os_ctx.prio_mask_lkp[task->curr_prio];
        os_ctx.prio_grp_cluster_rdy |= os_ctx.prio_mask_lkp[os_ctx.prio_indx_lkp[task->curr_prio]];
    }
    else
    {
        task->prev_in_prioq = NYA_NULL;
        task->next_in_prioq = os_ctx.prioq_l[task->curr_prio].first;
        os_ctx.prioq_l[task->curr_prio].first->prev_in_prioq = task;
        os_ctx.prioq_l[task->curr_prio].first = task;
    }
}
