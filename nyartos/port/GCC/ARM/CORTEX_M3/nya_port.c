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

void task_exit(void);
void task_exit(void)
{
    while (1)
    {}
}

nya_stack_t* nya_port_init_stack(nya_task_func_t entry_func,
                                 nya_stack_t *stack_ptr,
                                 nya_stack_t stack_size)
{
    stack_ptr = &stack_ptr[stack_size];
    stack_ptr = (nya_stack_t *)((nya_stack_t)(stack_ptr) & 0xfffffff8U); /* align to 8 bytes */
    *(--stack_ptr) = (nya_stack_t)0x01000000UL;                          /* xPSR : set thumb state */
    *(--stack_ptr) = (nya_stack_t)entry_func & (nya_stack_t)0xfffffffe;  /* task entry */
    *(--stack_ptr) = (nya_stack_t)task_exit & (nya_stack_t)0xfffffffe;   /* LR */
    stack_ptr -= 4;                                                      /* R12, R3 - R1 */
    *(--stack_ptr) = (nya_stack_t)0x00000000UL;                          /* R0 TODO: add task param */
    stack_ptr -= 8;                                                      /* R11 - R4 */

    return stack_ptr;
}

static nya_stack_t nya_exception_stack[256];  /* allocate 1KB as exception stack */

void nya_port_startup(void)
{
    __asm volatile
    (
        "cpsid i \n"  /* disable interrupts */
        :
        :
        : "memory"
    );

    os_ctx.is_running = NYA_TRUE;
    os_ctx.curr_task = os_ctx.next_task;
    NYA_PORT_NVIC_PENDSV_PRIO_REG = NYA_PORT_NVIC_PENDSV_PRIO_VAL;
    nya_stack_t *exception_stack;

    /* get top of stack and align to 8 bytes */
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
        :
        : "r" (exception_stack), "r" (os_ctx.curr_task)
        : "memory"
    );
}

/* ------------------------------------------------------------------------------ */
/* Context Switching */
/* ------------------------------------------------------------------------------ */

void nya_port_context_switch()
{
    NYA_PORT_NVIC_INT_CTRL_REG = NYA_PORT_NVIC_PENDSVSET_BIT;
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

/* ------------------------------------------------------------------------------ */
/* ISR Handlers */
/* ------------------------------------------------------------------------------ */

void nya_port_systick_handler(void)
{
    nya_enter_isr();

    nya_time_systick();

    nya_exit_isr();
}

void nya_port_pendsv_handler(void)
{
    __asm volatile
    (
        "cpsid i                  \n"  /* disable interrupts */
        "mov r0, %1               \n"
        "msr basepri, r0          \n"
        "dsb                      \n"
        "isb                      \n"
        "cpsie i                  \n"
        "                         \n"
        "mrs r0, psp              \n"  /* load old process stack pointer */
        "stmdb r0!, {r4-r11}      \n"  /* push registers */
        "                         \n"
        "mov r1, %2               \n"  /* load address of the current task */
        "str r0, [r1]             \n"  /* store stack pointer to current task */
        "mov r2, %3               \n"  /* load address of the next task */
        "ldr r0, [r2]             \n"  /* load next stack pointer */
        "                         \n"
        "mov %0, r2               \n"  /* nya_sys_ctx.curr_task = nya_sys_ctx.next_task; */
        "                         \n"
        "ldmia r0!, {r4-r11}      \n"  /* pop registers */
        "msr psp, r0              \n"  /* load new process stack pointer */
        "                         \n"
        "cpsid i                  \n"  /* enable interrupts */
        "mov r0, #0               \n"
        "msr basepri, r0          \n"
        "dsb                      \n"
        "isb                      \n"
        "cpsie i                  \n"
        "bx lr                    \n"
        : "=r" (os_ctx.curr_task)
        : "i" (NYA_PORT_BASEPRI_VAL), "r" (os_ctx.curr_task), "r" (os_ctx.next_task)
        : "memory"
    );
}
