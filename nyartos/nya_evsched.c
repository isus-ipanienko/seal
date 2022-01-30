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
/* Context */
/* ------------------------------------------------------------------------------ */

typedef struct
{
    nya_tcb_group_t tcb_groups[NYA_CFG_GROUPS];
    nya_u8_t group_ready[NYA_CFG_GROUPS];
} evsched_ctx_t;

static evsched_ctx_t ctx =
{
    .tcb_groups =
    {
#define NYA_GROUP(_priority, _mode)             \
    [_priority] =                               \
    {                                           \
        .mode   = _mode,                        \
        .tcb    =                               \
        {                                       \
            NYA_GROUP_##_priority##_DEFINITIONS \
        }                                       \
    },
#define NYA_TASK(_priority) \
    [_priority] =           \
    {                       \
    },
    NYA_GROUP_DEFINITIONS
#undef NYA_GROUP
#undef NYA_TASK
    }
};

/* ------------------------------------------------------------------------------ */
/* Private Prototypes */
/* ------------------------------------------------------------------------------ */

static void _task_switch(void);

/* ------------------------------------------------------------------------------ */
/* Private Declarations */
/* ------------------------------------------------------------------------------ */

static void _task_switch(void)
{
    NYA_DECLARE_CRITICAL();
    NYA_ENTER_CRITICAL();

    /*TODO: task switching */

    NYA_EXIT_CRITICAL();
}
