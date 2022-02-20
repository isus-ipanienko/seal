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

nya_bool_t nya_scheduler_set_next_task(void)
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

void nya_scheduler_switch(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    if (nya_scheduler_set_next_task())
    {
        NYA_CTX_SWITCH();
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
        nya_panic();
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
        nya_panic();
    }

    os_ctx.isr_nesting_cnt--;

    if (nya_scheduler_set_next_task())
    {
        NYA_CTX_SWITCH_FROM_ISR();
    }

    NYA_EXIT_CRITICAL();
}
