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

#include "nyartos_private_port.h"

/* ------------------------------------------------------------------------------ */
/* Context */
/* ------------------------------------------------------------------------------ */

typedef struct
{
    nya_size_t critical_nesting;
} port_ctx_t;

static port_ctx_t ctx =
{
    .critical_nesting = 0,
};

/* ------------------------------------------------------------------------------ */
/* Critical Section */
/* ------------------------------------------------------------------------------ */

nya_reg_t nya_port_enter_critical()
{
    nya_reg_t new_basepri;
    nya_reg_t old_basepri;

    __asm volatile
    (
        "cpsid i            \n" \
        "mrs %1, basepri    \n" \
        "mov %0, %2         \n" \
        "msr basepri, %0    \n" \
        "dsb                \n" \
        "isb                \n" \
        "cpsie i            \n" \
        : "=r" (new_basepri), "=r" (old_basepri)
        : "i" (NYA_PORT_BASEPRI_VAL)
        : "memory"
    );

    return old_basepri;
}

void nya_port_exit_critical(nya_reg_t restored_basepri)
{
    __asm volatile
    (
        "cpsid i            \n" \
        "msr basepri, %0    \n" \
        "dsb                \n" \
        "isb                \n" \
        "cpsie i            \n" \
        :
        : "r" (restored_basepri)
        : "memory"
    );
}
