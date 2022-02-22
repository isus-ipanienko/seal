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
/* Memory Allocation */
/* ------------------------------------------------------------------------------ */

#define NYA_CFG_KERNEL_EVENT_CNT    4U /**< max: nya_size_t max value */

/* ------------------------------------------------------------------------------ */
/* Task Definitions */
/* ------------------------------------------------------------------------------ */

/*
 *  NYA_TASK(_name,
 *           _priority,
 *           _stack_size,
 *           _entry_func,
 *           _entry_func_param)
 */
#define NYA_TASK_DEFINITIONS \
    NYA_TASK(LED,            \
             0,              \
             512,            \
             led_entry,      \
             NYA_NULL)       \
    NYA_TASK(PRINT,          \
             0,              \
             512,            \
             print_entry,    \
             NYA_NULL)       \
    NYA_TASK(IDLE,           \
             1,              \
             512,            \
             idle_entry,     \
             NYA_NULL)

/* ------------------------------------------------------------------------------ */
/* Entry Function Prototypes */
/* ------------------------------------------------------------------------------ */

#define NYA_TASK(_name,             \
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
#define NYA_TASK(_name,             \
                 _priority,         \
                 _stack_size,       \
                 _entry_func,       \
                 _entry_func_param) \
    NYA_TASK_ID_##_name,
    NYA_TASK_DEFINITIONS
#undef NYA_TASK
    NYA_TASK_ID_TOP,
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
#define NYA_TASK(_name,             \
                 _priority,         \
                 _stack_size,       \
                 _entry_func,       \
                 _entry_func_param) \
    NYA_PRIORITY_LEVEL_##_name = _priority,
    NYA_TASK_DEFINITIONS
#undef NYA_TASK
    NYA_PRIORITY_LEVEL_CNT,
} nya_priority_level_t;
/* *INDENT-ON* */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYA_TASK_CONFIG_H */
