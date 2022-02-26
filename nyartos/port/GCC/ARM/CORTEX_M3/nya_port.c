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
/* Stack */
/* ------------------------------------------------------------------------------ */

nya_stack_t* nya_port_init_stack(nya_task_func_t entry_func,
                                 nya_stack_t *stack_ptr,
                                 nya_stack_t stack_size,
                                 void *param)
{
    nya_stack_t *ptr = &stack_ptr[stack_size];

    ptr = (nya_stack_t *)((nya_stack_t)(ptr) & 0xfffffff8U);              /* align to 8 bytes */
    *(--ptr) = (nya_stack_t)0x01000000UL;                                 /* xPSR : set thumb state */
    *(--ptr) = (nya_stack_t)entry_func & (nya_stack_t)0xfffffffe;         /* task entry */
    *(--ptr) = (nya_stack_t)nya_core_task_exit & (nya_stack_t)0xfffffffe; /* LR */
    *(--ptr) = (nya_stack_t)0x0000000CUL;                                 /* R12 */
    *(--ptr) = (nya_stack_t)0x00000003UL;                                 /* R3 */
    *(--ptr) = (nya_stack_t)0x00000002UL;                                 /* R2 */
    *(--ptr) = (nya_stack_t)0x00000001UL;                                 /* R1 */
    *(--ptr) = (nya_stack_t)param;                                        /* R0 */
    *(--ptr) = (nya_stack_t)0x0000000BUL;                                 /* R11 */
    *(--ptr) = (nya_stack_t)0x0000000AUL;                                 /* R10 */
    *(--ptr) = (nya_stack_t)0x00000009UL;                                 /* R9 */
    *(--ptr) = (nya_stack_t)0x00000008UL;                                 /* R8 */
    *(--ptr) = (nya_stack_t)0x00000007UL;                                 /* R7 */
    *(--ptr) = (nya_stack_t)0x00000006UL;                                 /* R6 */
    *(--ptr) = (nya_stack_t)0x00000005UL;                                 /* R5 */
    *(--ptr) = (nya_stack_t)0x00000004UL;                                 /* R4 */

    nya_stack_t *ret = ptr;

    while (ptr-- != stack_ptr)
    {
        *ptr = 0xdeadbeef;
    }

    return ret;
}

static nya_stack_t nya_exception_stack[256];  /* allocate 1KB as exception stack */

void nya_port_startup(void)
{
    __asm volatile
    (
        "cpsid i  \n"  /* disable interrupts */
        ".align 4 \n"
        :
        :
        : "memory"
    );

    nya_curr_task = nya_next_task;
    NYA_PORT_NVIC_PENDSV_PRIO_REG = NYA_PORT_NVIC_PENDSV_PRIO_VAL;
    os_ctx.is_running = NYA_TRUE;

    /* get top of stack and align to 8 bytes */
    nya_stack_t *exception_stack;

    exception_stack = &nya_exception_stack[NYA_ARRAY_SIZE(nya_exception_stack)];
    exception_stack = (nya_stack_t *)((nya_stack_t)exception_stack & 0xfffffff8U);

    __asm volatile
    (
        "mov r0, %0                   \n"  /* load exception stack as msp */
        "msr msp, r0                  \n"
        "                             \n"
        "mov r2, %1                   \n"  /* load address of the current task */
        "ldr r3, [r2]                 \n"  /* load new process stack pointer */
        "msr psp, r3                  \n"  /* load it to psp */
        "                             \n"
        "mrs r0, control              \n"  /* set psp as the current stack pointer */
        "orr r0, r0, #2               \n"
        "msr control, r0              \n"
        "isb                          \n"
        "                             \n"
        "ldmia sp!, {r4-r11}          \n"  /* restore the remaining registers */
        "ldmia sp!, {r0-r3, r12, r14} \n"
        "ldmia sp!, {r1, r2}          \n"
        "orr r1, #1                   \n"  /* ensure r1[0] is set */
        "                             \n"
        "cpsie i                      \n"  /* enable interrupts */
        "bx r1                        \n"  /* start task */
        ".align 4                     \n"
        :
        : "r" (exception_stack), "r" (nya_curr_task)
        : "memory"
    );
}

/* ------------------------------------------------------------------------------ */
/* ISR Handlers */
/* ------------------------------------------------------------------------------ */

void nya_port_systick_handler(void)
{
    nya_enter_isr();

    nya_time_systick();

    nya_exit_isr();
}
