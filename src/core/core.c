#include "private.h"

os_tcb_t *volatile os_curr_task = OS_NULL;
os_tcb_t *volatile os_next_task = OS_NULL;

#define OS_TASK(_id, _priority, _stack_size, _entry_func, _entry_func_param)   \
  static os_stack_t os_stack_##_id[OS_PORT_BYTES_TO_SECTORS(_stack_size)];
OS_TASK_DEFINITIONS
#undef OS_TASK

os_ctx_t os_ctx = {0};

/**
 * @brief   Initializes a single task control block.
 * @param id
 * @param base_prio
 * @param stack_size
 */
static void _init_tcb(os_size_t id, os_u8_t base_prio, os_stack_t stack_size,
                      os_stack_t *stack_base, os_task_func_t entry_func,
                      void *entry_func_param) {
  os_ctx.tcbs[id].tid = id;
  os_ctx.tcbs[id].base_prio = base_prio;
  os_ctx.tcbs[id].curr_prio = base_prio;
  os_ctx.tcbs[id].stack_ptr =
      os_port_init_stack(entry_func, stack_base, stack_size, entry_func_param);
#if OS_CFG_ENABLE_STATS
  os_ctx.tcbs[id].stack_size = stack_size;
  os_ctx.tcbs[id].stack_end = &stack_base[stack_size - 1];
#endif /* if OS_CFG_ENABLE_STATS */
  os_queue_push(&os_ctx.tcbs[id], &os_ctx.priorities[base_prio]);
  OS_PRIORITY_READY(base_prio);
  os_ctx.tcbs[id].state = OS_TASK_READY;
}

/**
 * @brief Sets @c os_next_task to point at the first ready task with the
 * highest priority.
 *
 * @return OS_TRUE - a context switch is needed; OS_FALSE - no context switch
 * is needed
 */
os_bool_t _set_next_task(void) {
  if (os_ctx.isr_nesting_cnt == 0) {
    os_u8_t highest_priority = OS_GET_HIGHEST_PRIORITY(os_ctx.ready_priorities);

    if (os_ctx.priorities[highest_priority].first != os_next_task) {
      os_next_task = os_ctx.priorities[highest_priority].first;

      return OS_TRUE;
    }
  }

  return OS_FALSE;
}

void os_panic(os_error_t reason) {
  OS_DISABLE_INTERRUPTS();
  os_panic_hook(reason);
  while (1) {
  }
}

void os_task_exit(void) {
  os_task_exit_hook();
  os_panic(OS_TASK_EXITED);
}

void os_schedule(void) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  if (_set_next_task()) {
    OS_CTX_SWITCH();
  }
  OS_EXIT_CRITICAL();
}

void os_systick(void) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  /* TODO: create a waiting list with delayed tasks */
  for (os_size_t id = 0; id < OS_TASK_ID_CNT; id++) {
    if (os_ctx.tcbs[id].delay) {
      if (--os_ctx.tcbs[id].delay == 0) {
        switch (os_ctx.tcbs[id].state) {
        case OS_TASK_WAITING_FOR_EVENT:
          os_event_timeout(&os_ctx.tcbs[id]);
          break;
        case OS_TASK_ASLEEP:
          break;
        default:
          os_panic(OS_ERROR);
        }
        os_queue_push(&os_ctx.tcbs[id],
                      &os_ctx.priorities[os_ctx.tcbs[id].curr_prio]);
        OS_PRIORITY_READY(os_ctx.tcbs[id].curr_prio);
        os_ctx.tcbs[id].state = OS_TASK_READY;
      }
    }
  }
  OS_EXIT_CRITICAL();
}

void os_queue_push(os_tcb_t *task, os_queue_t *queue) {
  OS_ASSERT((task != OS_NULL) && (queue != OS_NULL), OS_NULL_PARAM);
  if (queue->first == OS_NULL) {
    task->prev = OS_NULL;
    task->next = OS_NULL;
    queue->first = task;
    queue->last = task;
  } else {
    task->prev = queue->last;
    task->next = OS_NULL;
    queue->last->next = task;
    queue->last = task;
  }
}

void os_queue_pop(os_queue_t *queue) {
  OS_ASSERT((queue->first != OS_NULL), OS_NULL_PARAM);
  if (queue->first->next == OS_NULL) {
    queue->first = OS_NULL;
    queue->last = OS_NULL;
  } else {
    queue->first = queue->first->next;
  }
}

void os_queue_remove(os_tcb_t *task, os_queue_t *queue) {
  if (queue->first == task) {
    queue->first = task->next;
  }
  if (queue->last == task) {
    queue->last = task->prev;
  }
  if (task->prev != OS_NULL) {
    task->prev->next = task->next;
  }
  if (task->next != OS_NULL) {
    task->next->prev = task->prev;
  }
}

void os_update_priority(os_tcb_t *task, os_u8_t new_prio) {
  if (task->curr_prio == new_prio) {
    return;
  }
  os_queue_remove(task, &os_ctx.priorities[task->curr_prio]);
  OS_PRIORITY_UNREADY(task->curr_prio);
  task->curr_prio = new_prio;
  os_queue_push(task, &os_ctx.priorities[task->curr_prio]);
  OS_PRIORITY_READY(task->curr_prio);
}

void os_enter_isr(void) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  OS_ASSERT((os_ctx.isr_nesting_cnt != 255), OS_ISR_OVERFLOW);
  os_ctx.isr_nesting_cnt++;
  OS_EXIT_CRITICAL();
}

void os_exit_isr(void) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  OS_ASSERT((os_ctx.isr_nesting_cnt != 0), OS_ISR_UNDERFLOW);
  os_ctx.isr_nesting_cnt--;
  if (_set_next_task()) {
    OS_CTX_SWITCH_FROM_ISR();
  }
  OS_EXIT_CRITICAL();
}

void os_sleep(os_size_t ticks) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  os_curr_task->state = OS_TASK_ASLEEP;
  os_curr_task->delay = ticks;
  os_queue_pop(&os_ctx.priorities[os_curr_task->curr_prio]);
  OS_PRIORITY_UNREADY(os_curr_task->curr_prio);
  OS_EXIT_CRITICAL();
  os_schedule();
}

void os_init() {
#define OS_TASK(_id, _priority, _stack_size, _entry_func, _entry_func_param)   \
  _init_tcb(_id, _priority, OS_PORT_BYTES_TO_SECTORS(_stack_size),             \
            os_stack_##_id, _entry_func, _entry_func_param);
  OS_TASK_DEFINITIONS
#undef OS_TASK

#define OS_MUTEX(_id) os_event_init(_id, OS_EVENT_MUTEX, 0);
  OS_MUTEX_DEFINITIONS
#undef OS_MUTEX

#define OS_SEMAPHORE(_id, _count)                                              \
  os_event_init(_id, OS_EVENT_SEMAPHORE, _count);
  OS_SEMAPHORE_DEFINITIONS
#undef OS_SEMAPHORE

  if (_set_next_task()) {
    os_port_startup();
  }
  os_panic(OS_STARTUP_EXITED);
}
