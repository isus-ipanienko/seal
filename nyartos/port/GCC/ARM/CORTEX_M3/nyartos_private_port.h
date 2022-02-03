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

#ifndef NYARTOS_PRIVATE_PORT_H
#define NYARTOS_PRIVATE_PORT_H

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* ------------------------------------------------------------------------------ */
/* CPU Registers */
/* ------------------------------------------------------------------------------ */

#define NYA_PORT_NVIC_INT_CTRL_REG     ((volatile nya_reg_t *)0xe000ed04)
#define NYA_PORT_NVIC_PENDSVSET_BIT    (1UL << 28UL)

/* ------------------------------------------------------------------------------ */
/* Context Switching */
/* ------------------------------------------------------------------------------ */

#define NYA_CTX_SWITCH()             nya_port_context_switch()
#define NYA_CTX_SWITCH_FROM_ISR()    nya_port_context_switch()

/** @brief This function triggers PendSV. */
void nya_port_context_switch(void);

/* ------------------------------------------------------------------------------ */
/* Critical Section */
/* ------------------------------------------------------------------------------ */

#define NYA_PORT_MAX_SYSCALL_INT_PRIORITY    4U
#define NYA_PORT_NVIC_OFFSET                 4U
#define NYA_PORT_BASEPRI_VAL                 (NYA_PORT_MAX_SYSCALL_INT_PRIORITY << NYA_PORT_NVIC_OFFSET)

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYARTOS_PRIVATE_PORT_H */
