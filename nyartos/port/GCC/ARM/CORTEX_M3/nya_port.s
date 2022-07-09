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

.extern  nya_curr_task
.extern  nya_next_task

.global  nya_port_pendsv_handler
.global  nya_port_context_switch
.global  nya_port_enter_critical
.global  nya_port_exit_critical

.equ NYA_PORT_NVIC_INT_CTRL_REG,     0xE000ED04
.equ NYA_PORT_NVIC_PENDSVSET_BIT,    0x10000000
.equ NYA_PORT_BASEPRI_VAL,           0x40

/* TODO: this
.equ NYA_PORT_MAX_SYSCALL_INT_PRIORITY,    0x4U
.equ NYA_PORT_NVIC_OFFSET,                 0x4U
.equ NYA_PORT_BASEPRI_VAL,                 (NYA_PORT_MAX_SYSCALL_INT_PRIORITY << NYA_PORT_NVIC_OFFSET)
*/

.text
.align 4
.thumb
.syntax unified

/* ------------------------------------------------------------------------------ */
/* Context Switching */
/* ------------------------------------------------------------------------------ */

.thumb_func
nya_port_context_switch:
    ldr r0, =NYA_PORT_NVIC_INT_CTRL_REG
    ldr r1, =NYA_PORT_NVIC_PENDSVSET_BIT
    str r1, [r0]
    dsb
    isb
    bx lr

/* ------------------------------------------------------------------------------ */
/* Critical Section */
/* ------------------------------------------------------------------------------ */

.thumb_func
nya_port_enter_critical:
    cpsid i
    push {r1}
    ldr r0, =NYA_PORT_BASEPRI_VAL
    mrs r1, basepri
    msr basepri, r0
    dsb
    isb
    mov r0, r1
    pop {r1}
    cpsie i
    bx lr

.thumb_func
nya_port_exit_critical:
    cpsid i
    msr basepri, r0
    dsb
    isb
    cpsie i
    bx lr

/* ------------------------------------------------------------------------------ */
/* ISR Handlers */
/* ------------------------------------------------------------------------------ */

.thumb_func
nya_port_pendsv_handler:
    cpsid i                  /* disable interrupts */
    ldr r0, =NYA_PORT_BASEPRI_VAL
    msr basepri, r0
    dsb
    isb
    cpsie i

    mrs r0, psp              /* load old process stack pointer */
    stmdb r0!, {r4-r11}      /* push registers */

    ldr r1, =nya_curr_task   /* get address of current taskptr */
    ldr r2, [r1]             /* get address of current task and stack pointer */
    str r0, [r2]             /* store stack pointer to current task */

    ldr r3, =nya_next_task   /* get address of next taskptr */
    ldr r2, [r3]             /* get address of next task and stack pointer */
    str r2, [r1]             /* set next task as current task */

    ldr r0, [r2]             /* get value of next stack pointer */
    ldmia r0!, {r4-r11}      /* pop registers */
    msr psp, r0              /* load new process stack pointer */

    cpsid i                  /* enable interrupts */
    mov r0, #0
    msr basepri, r0
    dsb
    isb
    cpsie i
    bx lr

.end
