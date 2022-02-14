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

#ifndef NYA_TASK_CONFIG_H
#define NYA_TASK_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

typedef void (*nya_task_func_t)(void *);

/* *INDENT-OFF* */

/*
    NYA_TASK(_priority,
             _stack_size,
             _name, 
             _entry_func)
*/
#define NYA_TASK_DEFINITIONS              \
    NYA_TASK(0, 512, LED, led_entry)      \
    NYA_TASK(0, 512, PRINT, print_entry)  \
    NYA_TASK(1, 512, IDLE, idle_entry)

#define NYA_TASK(_priority,        \
                 _stack_size,      \
                 _name,            \
                 _entry_func)      \
    void _entry_func(void *);
    NYA_TASK_DEFINITIONS
#undef NYA_TASK

typedef enum
{
#define NYA_TASK(_priority,   \
                 _stack_size, \
                 _name,       \
                 _entry_func) \
    NYA_TASK_ID_##_name,
    NYA_TASK_DEFINITIONS
#undef NYA_TASK
} nya_task_id_t;

/* *INDENT-ON* */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYA_TASK_CONFIG_H */
