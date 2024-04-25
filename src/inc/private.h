#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "seal.h"
#include "port_private.h"

#define OS_ARRAY_SIZE(_array) (sizeof(_array) / sizeof(_array[0]))
#define OS_ASSERT(_condition, _id)                                             \
  do {                                                                         \
    if (!(_condition))                                                         \
      os_panic((_id));                                                         \
  } while (0);

/**
 * @brief Type for task entry functions.
 */
typedef void (*os_task_func_t)(void *);

/**
 * @brief Wait return type.
 */
typedef enum {
  OS_WAIT_RET_OK,
  OS_WAIT_RET_TIMEOUT,
} os_wait_ret_t;

/**
 * @brief Task states.
 */
typedef enum {
  OS_TASK_READY,
  OS_TASK_RUNNING,
  OS_TASK_ASLEEP,
  OS_TASK_WAITING_FOR_EVENT,
} os_task_state_t;

typedef enum {
  OS_EVENT_UNINITIALIZED = 0,
  OS_EVENT_MUTEX,
  OS_EVENT_SEMAPHORE,
  OS_EVENT_TOP,
} os_event_type_t;

/**
 * @brief Queue type.
 */
typedef struct {
  struct os_tcb_t *first;
  struct os_tcb_t *last;
} os_queue_t;

/**
 * @brief Event type.
 */
typedef struct {
  os_event_type_t type;
  struct os_tcb_t *holder;
  os_queue_t queue;
  os_size_t count;
} os_event_t;

/**
 * @brief Task control block type.
 */
typedef struct os_tcb_t {
  os_stack_t *stack_ptr;

  os_size_t delay;
  os_task_state_t state;
  os_task_id_t tid;

  os_u8_t base_prio;
  os_u8_t curr_prio;

  struct os_tcb_t *next;
  struct os_tcb_t *prev;
  os_event_t *wait_event;
  os_wait_ret_t wait_return;

#if OS_CFG_ENABLE_MESSAGE_QUEUES
  os_msgq_t msgq;
#endif

#if OS_CFG_ENABLE_STATS
  os_stack_t *stack_end;
  os_size_t stack_size;
#endif
} os_tcb_t;

/**
 * @brief System context type.
 */
typedef struct {
  os_bool_t is_running;

  os_tcb_t tcbs[OS_TASK_ID_CNT];
  os_event_t events[OS_EVENT_ID_CNT];
  os_queue_t priorities[OS_PRIORITY_LEVEL_CNT];

  os_u32_t ready_priorities;
  os_u8_t isr_nesting_cnt;
} os_ctx_t;

extern os_tcb_t *volatile os_curr_task;
extern os_tcb_t *volatile os_next_task;
extern os_ctx_t os_ctx;

/**
 * @brief This function is called when a task exits.
 */
void os_task_exit(void);

/**
 * @brief    Triggers a context switch.
 * @warning  Don't call it from an ISR, @c os_exit_isr() handles context
 * switching from ISRs.
 */
void os_schedule(void);

/**
 * @brief   Increments the systick.
 * @note    This function contains a critical section.
 */
void os_systick(void);

/**
 * @brief Initializes a mutex.
 * @param [in] id - id of the mutex
 */
void os_event_init(os_event_id_t id, os_event_type_t type, os_u32_t count);

/**
 * @brief Times out a task and removes it from an event waiting list.
 * @param [in] task - timed out task
 */
void os_event_timeout(os_tcb_t *task);

/**
 * @brief   Pushes a task to a queue.
 * @note    Call this from within a critical section.
 */
void os_queue_push(os_tcb_t *task, os_queue_t *queue);

/**
 * @brief   Pops a queue.
 * @note    Call this from within a critical section.
 */
void os_queue_pop(os_queue_t *queue);

/**
 * @brief   Removes a task from a queue.
 * @note    Call this from within a critical section.
 */
void os_queue_remove(os_tcb_t *task, os_queue_t *queue);

/**
 * @brief   Updates a task priority.
 */
void os_update_priority(os_tcb_t *task, os_u8_t new_prio);

/**
 * @brief   Initializes a task's stack.
 * @note    It needs to be implemented in @c os_port.c or @c os_port.s
 * @note    If the stack needs to be aligned, it should be handled here.
 *
 * @param[in] entry_func - pointer to the task's entry function
 * @param[in] stack_ptr - pointer to the beginning of the task's stack
 * @param[in] stack_size - size of the stack
 * @param[in] param - task entry function parameter
 * @return os_stack_t* - pointer of the initialized stack
 */
os_stack_t *os_port_init_stack(os_task_func_t entry_func, os_stack_t *stack_ptr,
                               os_stack_t stack_size, void *param);

/**
 * @brief Initializes the system and starts scheduling.
 */
void os_port_startup(void);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

