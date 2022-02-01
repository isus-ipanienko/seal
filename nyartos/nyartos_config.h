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

#ifndef NYARTOS_CONFIG_H
#define NYARTOS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* ------------------------------------------------------------------------------ */
/* Task Settings */
/* ------------------------------------------------------------------------------ */

#define NYA_CFG_ENABLE_STATS             0U
#define NYA_CFG_ENABLE_MESSAGE_QUEUES    0U

/* ------------------------------------------------------------------------------ */
/* Tasks */
/* ------------------------------------------------------------------------------ */

#define NYA_CFG_PRIORITY_LEVELS    2U /**< Cannot be more than 64 */
#define NYA_CFG_TASK_CNT           8U /**< You can't use more than 256 tasks per priority level */

/**fmt-off**/
/*
    NYA_TASK(_priority)
*/
#define NYA_TASK_DEFINITIONS    \
    NYA_TASK(0)                 \
    NYA_TASK(1)                 \
    NYA_TASK(1)                 \
    NYA_TASK(1)
/**fmt-on**/

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYARTOS_CONFIG_H */
