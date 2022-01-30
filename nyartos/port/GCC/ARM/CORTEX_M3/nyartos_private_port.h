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
/* Includes */
/* ------------------------------------------------------------------------------ */

#include "nyartos_port.h"
#include "nyartos_config.h"

/* ------------------------------------------------------------------------------ */
/* CPU Settings */
/* ------------------------------------------------------------------------------ */

#define NYA_PORT_USING_FPU_UNIT    0U
#define NYA_PORT_STACK_DIR         (-1)
#define NYA_PORT_TICK_IS_ATOMIC    1U

/* ------------------------------------------------------------------------------ */
/* Critical Section */
/* ------------------------------------------------------------------------------ */

#ifndef NYA_CFG_MAX_SYSCALL_INT_PRIORITY
#error "NYA_CFG_MAX_SYSCALL_INT_PRIORITY is not defined in nya_config.h!"
#endif /* ifndef NYA_CFG_MAX_SYSCALL_INT_PRIORITY */

#define NYA_PORT_NVIC_OFFSET    4U
#define NYA_PORT_BASEPRI_VAL    (NYA_CFG_MAX_SYSCALL_INT_PRIORITY << NYA_PORT_NVIC_OFFSET)

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYARTOS_PRIVATE_PORT_H */
