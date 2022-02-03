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
/* Context Switching */
/* ------------------------------------------------------------------------------ */

void nya_port_pendsv_handler(void)
{
    __asm volatile
    (
        "cpsid i                  \n"  /* disable interrupts */
        "mov r0, %0               \n"
        "ldr r1, [r0]             \n"
        "msr basepri, r1          \n"
        "dsb                      \n"
        "isb                      \n"
        "cpsie i                  \n"
        "                         \n"
        "stmdb r0!, {r4-r11, r14} \n"  /* push registers */
        "                         \n"
        "mov r1, %1               \n"  /* load &nya_sys_ctx.curr_task */
        "ldr r2, [r1]             \n"  /* get address of current tcb */
        "str r0, [r2]             \n"  /* store stack pointer to current tcb->stack_pointer */
        "mov r2, %2               \n"  /* load &nya_sys_ctx.next_task */
        "ldr r3, [r2]             \n"  /* get address of next tcb */
        "str r3, [r1]             \n"  /* store adress of nya_sys_ctx.next_task to nya_sys_ctx.curr_task */
        "                         \n"
        "ldr r0, [r3]             \n"  /* load new process stack pointer */
        "ldmia r0!, {r4-r11, r14} \n"  /* pop registers */
        "msr psp, r0              \n"  /* load new process stack pointer */
        "                         \n"
        "cpsid i                  \n"  /* enable interrupts */
        "mov r0, #0               \n"
        "msr basepri, r0          \n"
        "dsb                      \n"
        "isb                      \n"
        "cpsie i                  \n"
        :
        : "i" (NYA_PORT_BASEPRI_VAL), "r" (nya_sys_ctx.curr_task), "r" (nya_sys_ctx.next_task)
        : "memory"
    );
}

void nya_port_context_switch()
{
    *NYA_PORT_NVIC_INT_CTRL_REG = NYA_PORT_NVIC_PENDSVSET_BIT;
    __asm volatile
    (
        "dsb \n" \
        "isb \n" \
        :
        :
        : "memory"
    );
}

/* ------------------------------------------------------------------------------ */
/* Critical Section */
/* ------------------------------------------------------------------------------ */

nya_reg_t nya_port_enter_critical()
{
    nya_reg_t new_basepri;
    nya_reg_t old_basepri;

    __asm volatile
    (
        "cpsid i            \n"
        "mrs %1, basepri    \n"
        "mov %0, %2         \n"
        "msr basepri, %0    \n"
        "dsb                \n"
        "isb                \n"
        "cpsie i            \n"
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
        "cpsid i            \n"
        "msr basepri, %0    \n"
        "dsb                \n"
        "isb                \n"
        "cpsie i            \n"
        :
        : "r" (restored_basepri)
        : "memory"
    );
}
