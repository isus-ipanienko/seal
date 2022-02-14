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

void nya_priority_pop(nya_u8_t priority)
{
    /* if (os_ctx.prioq_l[priority].count == 0) nya_panic(); ? */

    if (--os_ctx.prioq_l[priority].count == 0)
    {
        os_ctx.prio_grp_cluster_rdy &= ~os_ctx.prio_mask_lkp[priority];
        os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[priority]] &= ~os_ctx.prio_mask_lkp[priority];
        os_ctx.prioq_l[priority].first = NYA_NULL;
        os_ctx.prioq_l[priority].last = NYA_NULL;
    }
    else
    {
        os_ctx.prioq_l[priority].first = os_ctx.prioq_l[priority].first->next_in_prioq;
    }
}

void nya_priority_push(nya_size_t id,
                       nya_u8_t priority)
{
    if (os_ctx.prioq_l[priority].count == 0)
    {
        os_ctx.tcb_l[id].prev_in_prioq = NYA_NULL;
        os_ctx.tcb_l[id].next_in_prioq = NYA_NULL;
        os_ctx.prioq_l[priority].first = &os_ctx.tcb_l[id];
        os_ctx.prioq_l[priority].last = &os_ctx.tcb_l[id];
    }
    else
    {
        os_ctx.tcb_l[id].prev_in_prioq = os_ctx.prioq_l[priority].last;
        os_ctx.tcb_l[id].next_in_prioq = NYA_NULL;
        os_ctx.prioq_l[priority].last->next_in_prioq = &os_ctx.tcb_l[id];
        os_ctx.prioq_l[priority].last = &os_ctx.tcb_l[id];
    }

    os_ctx.prioq_l[priority].count++;
    os_ctx.prio_grp_cluster_rdy |= os_ctx.prio_mask_lkp[priority];
    os_ctx.prio_grp_rdy[os_ctx.prio_indx_lkp[priority]] |= os_ctx.prio_mask_lkp[priority];
}
