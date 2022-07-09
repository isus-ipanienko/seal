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

#ifndef NYARTOS_SYSTEM_CONFIG_H
#define NYARTOS_SYSTEM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* ------------------------------------------------------------------------------ */
/* Feature Settings */
/* ------------------------------------------------------------------------------ */

#define NYA_CFG_ENABLE_STATS             0U
#define NYA_CFG_ENABLE_MESSAGE_QUEUES    0U
#define NYA_CFG_ENABLE_MUTEXES           0U
#define NYA_CFG_ENABLE_SEMAPHORES        0U

/* ------------------------------------------------------------------------------ */
/* Config Asserts */
/* ------------------------------------------------------------------------------ */

#ifndef NYA_CFG_ENABLE_STATS
#error NYA_CFG_ENABLE_STATS must be defined!
#else /* ifndef NYA_CFG_ENABLE_STATS */
#if (NYA_CFG_ENABLE_STATS != 1U) && (NYA_CFG_ENABLE_STATS != 0U)
#error NYA_CFG_ENABLE_STATS needs to be either 1U or 0U!
#endif /* if (NYA_CFG_ENABLE_STATS != 1U) && (NYA_CFG_ENABLE_STATS != 0U) */
#endif /* ifndef NYA_CFG_ENABLE_STATS */

#ifndef NYA_CFG_ENABLE_MESSAGE_QUEUES
#error NYA_CFG_ENABLE_MESSAGE_QUEUES must be defined!
#else /* ifndef NYA_CFG_ENABLE_MESSAGE_QUEUES */
#if (NYA_CFG_ENABLE_MESSAGE_QUEUES != 1U) && (NYA_CFG_ENABLE_MESSAGE_QUEUES != 0U)
#error NYA_CFG_ENABLE_MESSAGE_QUEUES needs to be either 1U or 0U!
#endif /* if (NYA_CFG_ENABLE_MESSAGE_QUEUES != 1U) && (NYA_CFG_ENABLE_MESSAGE_QUEUES != 0U) */
#endif /* ifndef NYA_CFG_ENABLE_MESSAGE_QUEUES */

#ifndef NYA_CFG_ENABLE_MUTEXES
#error NYA_CFG_ENABLE_MUTEXES must be defined!
#else /* ifndef NYA_CFG_ENABLE_MUTEXES */
#if (NYA_CFG_ENABLE_MUTEXES != 1U) && (NYA_CFG_ENABLE_MUTEXES != 0U)
#error NYA_CFG_ENABLE_MUTEXES needs to be either 1U or 0U!
#endif /* if (NYA_CFG_ENABLE_MUTEXES != 1U) && (NYA_CFG_ENABLE_MUTEXES != 0U) */
#endif /* ifndef NYA_CFG_ENABLE_MUTEXES */

#ifndef NYA_CFG_ENABLE_SEMAPHORES
#error NYA_CFG_ENABLE_SEMAPHORES must be defined!
#else /* ifndef NYA_CFG_ENABLE_SEMAPHORES */
#if (NYA_CFG_ENABLE_SEMAPHORES != 1U) && (NYA_CFG_ENABLE_SEMAPHORES != 0U)
#error NYA_CFG_ENABLE_SEMAPHORES needs to be either 1U or 0U!
#endif /* if (NYA_CFG_ENABLE_SEMAPHORES != 1U) && (NYA_CFG_ENABLE_SEMAPHORES != 0U) */
#endif /* ifndef NYA_CFG_ENABLE_SEMAPHORES */

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYARTOS_SYSTEM_CONFIG_H */
