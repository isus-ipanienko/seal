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

/* ------------------------------------------------------------------------------ */
/* Kernel Event Definitions */
/* ------------------------------------------------------------------------------ */

#define NYA_MUTEX_DEFINITIONS \
    NYA_MUTEX(NYA_MUTEX_ID_FOO)

#define NYA_SEMAPHORE_DEFINITIONS       \
    NYA_SEMAPHORE(NYA_SEMAPHORE_ID_FOO, \
                  2)

/* ------------------------------------------------------------------------------ */
/* Kernel Event ID Enum */
/* ------------------------------------------------------------------------------ */

/* *INDENT-OFF* */
typedef enum
{
#define NYA_MUTEX(_id) \
    _id,
    NYA_MUTEX_DEFINITIONS
#undef NYA_MUTEX
#define NYA_SEMAPHORE(_id,    \
                      _count) \
    _id,
    NYA_SEMAPHORE_DEFINITIONS
#undef NYA_SEMAPHORE
    NYA_EVENT_ID_CNT,
} nya_event_id_t;
/* *INDENT-ON* */

/* ------------------------------------------------------------------------------ */
/* Task Definitions */
/* ------------------------------------------------------------------------------ */

/**
 * @brief   This macro is used to create all structures required by the tasks specified here.
 * @warning The task with the highest priority should appear last on the list. Else, the
 *          number of allocated priorities will be incorrect. Alternatively, you may remove
 *          the definition of @c nya_priority_level_t and define NYA_PRIORITY_LEVEL_CNT yourself.
 *          #define NYA_PRIORITY_LEVEL_CNT (max_priority + 1)
 *
 *  NYA_TASK(_id,               - Id of the task. It's used for inter-task communication.
 *           _priority,         - Base priority level of the task.
 *           _stack_size,       - Stack size allocated for the task [in bytes].
 *                                Note that it will be rounded down to a multiple of sizeof(nya_stack_t).
 *           _entry_func,       - Name of the entry function.
 *           _entry_func_param) - Pointer to the entry function parameter.
 */
#define NYA_TASK_DEFINITIONS    \
    NYA_TASK(NYA_TASK_ID_LED,   \
             0,                 \
             512,               \
             led_entry,         \
             NYA_NULL)          \
    NYA_TASK(NYA_TASK_ID_PRINT, \
             0,                 \
             512,               \
             print_entry,       \
             NYA_NULL)          \
    NYA_TASK(NYA_TASK_ID_IDLE,  \
             1,                 \
             512,               \
             idle_entry,        \
             NYA_NULL)

/* ------------------------------------------------------------------------------ */
/* Entry Function Prototypes */
/* ------------------------------------------------------------------------------ */

#define NYA_TASK(_id,               \
                 _priority,         \
                 _stack_size,       \
                 _entry_func,       \
                 _entry_func_param) \
    void _entry_func(void *);
NYA_TASK_DEFINITIONS
#undef NYA_TASK

/* ------------------------------------------------------------------------------ */
/* Task ID Enum */
/* ------------------------------------------------------------------------------ */

/* *INDENT-OFF* */
/**
 * @brief This enum is used for inter-task communication. Example member: NYA_TASK_ID_IDLE
 */
typedef enum
{
#define NYA_TASK(_id,               \
                 _priority,         \
                 _stack_size,       \
                 _entry_func,       \
                 _entry_func_param) \
    _id,
    NYA_TASK_DEFINITIONS
#undef NYA_TASK
    NYA_TASK_ID_CNT,
} nya_task_id_t;
/* *INDENT-ON* */

/* ------------------------------------------------------------------------------ */
/* Priority Enum */
/* ------------------------------------------------------------------------------ */

/* *INDENT-OFF* */
/**
 * @brief This enum is only used to calculate the amount of priority levels.
 */
typedef enum
{
#define NYA_TASK(_id,               \
                 _priority,         \
                 _stack_size,       \
                 _entry_func,       \
                 _entry_func_param) \
    NYA_PRIORITY_LEVEL_##_id = _priority,
    NYA_TASK_DEFINITIONS
#undef NYA_TASK
    NYA_PRIORITY_LEVEL_CNT,
} nya_priority_level_t;
/* *INDENT-ON* */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYA_TASK_CONFIG_H */
