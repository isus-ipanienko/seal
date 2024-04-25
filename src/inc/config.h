#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define OS_MUTEX_DEFINITIONS OS_MUTEX(OS_MUTEX_ID_FOO)

#define OS_SEMAPHORE_DEFINITIONS OS_SEMAPHORE(OS_SEMAPHORE_ID_FOO, 2)

/**
 * @brief   This macro is used to create all structures required by the tasks.
 * @warning The task with the highest priority should appear last on the list.
 *          Otherwise, the number of allocated priorities will be incorrect.
 *          Alternatively, you may remove the definition of os_priority_level_t
 *          and define OS_PRIORITY_LEVEL_CNT yourself.
 *          #define OS_PRIORITY_LEVEL_CNT (max_priority + 1)
 *
 *  _id,               - Id of the task. It's used for inter-task communication.
 *  _priority,         - Base priority level of the task.
 *  _stack_size,       - Stack size allocated for the task [in bytes].
 *                       Note that it will be rounded down to a multiple
 *                       of sizeof(os_stack_t).
 *  _entry_func,       - Name of the entry function.
 *  _entry_func_param  - Pointer to the entry function parameter.
 */
#define OS_TASK_DEFINITIONS                                                    \
  OS_TASK(OS_TASK_ID_IDLE, 0, 512, idle_entry, OS_NULL)                        \
  OS_TASK(OS_TASK_ID_LED, 1, 512, led_entry, OS_NULL)                          \
  OS_TASK(OS_TASK_ID_PRINT, 1, 512, print_entry, OS_NULL)

typedef enum {
#define OS_MUTEX(_id) _id,
  OS_MUTEX_DEFINITIONS
#undef OS_MUTEX
#define OS_SEMAPHORE(_id, _count) _id,
      OS_SEMAPHORE_DEFINITIONS
#undef OS_SEMAPHORE
          OS_EVENT_ID_CNT,
} os_event_id_t;

#define OS_TASK(_id, _priority, _stack_size, _entry_func, _entry_func_param)   \
  void _entry_func(void *);
OS_TASK_DEFINITIONS
#undef OS_TASK

/**
 * @brief This enum is used for inter-task communication. Example member:
 * OS_TASK_ID_IDLE
 */
typedef enum {
#define OS_TASK(_id, _priority, _stack_size, _entry_func, _entry_func_param)   \
  _id,
  OS_TASK_DEFINITIONS
#undef OS_TASK
      OS_TASK_ID_CNT,
} os_task_id_t;

/**
 * @brief This enum is used to calculate the amount of priority levels.
 */
typedef enum {
#define OS_TASK(_id, _priority, _stack_size, _entry_func, _entry_func_param)   \
  OS_PRIORITY_LEVEL_##_id = _priority,
  OS_TASK_DEFINITIONS
#undef OS_TASK
      OS_PRIORITY_LEVEL_CNT,
} os_priority_level_t;

#define OS_CFG_ENABLE_STATS 0U
#define OS_CFG_ENABLE_MESSAGE_QUEUES 0U
#define OS_CFG_ENABLE_MUTEXES 0U
#define OS_CFG_ENABLE_SEMAPHORES 0U

#ifndef OS_CFG_ENABLE_STATS
#error OS_CFG_ENABLE_STATS must be defined!
#else
#if (OS_CFG_ENABLE_STATS != 1U) && (OS_CFG_ENABLE_STATS != 0U)
#error OS_CFG_ENABLE_STATS needs to be either 1U or 0U!
#endif
#endif

#ifndef OS_CFG_ENABLE_MESSAGE_QUEUES
#error OS_CFG_ENABLE_MESSAGE_QUEUES must be defined!
#else
#if (OS_CFG_ENABLE_MESSAGE_QUEUES != 1U) && (OS_CFG_ENABLE_MESSAGE_QUEUES != 0U)
#error OS_CFG_ENABLE_MESSAGE_QUEUES needs to be either 1U or 0U!
#endif
#endif

#ifndef OS_CFG_ENABLE_MUTEXES
#error OS_CFG_ENABLE_MUTEXES must be defined!
#else
#if (OS_CFG_ENABLE_MUTEXES != 1U) && (OS_CFG_ENABLE_MUTEXES != 0U)
#error OS_CFG_ENABLE_MUTEXES needs to be either 1U or 0U!
#endif
#endif

#ifndef OS_CFG_ENABLE_SEMAPHORES
#error OS_CFG_ENABLE_SEMAPHORES must be defined!
#else
#if (OS_CFG_ENABLE_SEMAPHORES != 1U) && (OS_CFG_ENABLE_SEMAPHORES != 0U)
#error OS_CFG_ENABLE_SEMAPHORES needs to be either 1U or 0U!
#endif
#endif

#ifdef __cplusplus
}
#endif
