#include "private.h"

/**
 * @brief   Gets the last highest priority task in a mutex queue. Tasks are
 * always inserted at the beginning of a queue, so the last task with a given
 * priority is waiting for the longest time.
 * @param   [in] event - pointer to a mutex struct
 * @return  os_tcb_t* - pointer to the task
 */
static os_tcb_t *_mutex_get_next(const os_event_t *const event);

static os_tcb_t *_mutex_get_next(const os_event_t *const event) {
  if (event->queue.first == OS_NULL) {
    return OS_NULL;
  }
  os_tcb_t *high_prio_task = event->queue.first;
  os_tcb_t *next_task = high_prio_task->next;
  while (next_task != OS_NULL) {
    if (next_task->curr_prio >= high_prio_task->curr_prio) {
      high_prio_task = next_task;
    }
    next_task = next_task->next;
  }
  return high_prio_task;
}

void os_event_init(os_event_id_t id, os_event_type_t type, os_u32_t count) {
  OS_ASSERT((os_ctx.events[id].type == OS_EVENT_UNINITIALIZED),
            OS_EVENT_INITIALIZED);
  os_ctx.events[id].type = type;
  os_ctx.events[id].count = count;
}

void os_event_timeout(os_tcb_t *task) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  OS_ASSERT((task->wait_event->type != OS_EVENT_UNINITIALIZED) &&
                (task->wait_event->type < OS_EVENT_TOP),
            OS_WRONG_EVENT);
  task->wait_return = OS_WAIT_RET_TIMEOUT;
  os_queue_remove(task, &task->wait_event->queue);
  if (task->wait_event->type == OS_EVENT_MUTEX) {
    os_tcb_t *holder = task->wait_event->holder;
    os_tcb_t *high_prio_task = _mutex_get_next(task->wait_event);
    os_update_priority(holder, (high_prio_task != OS_NULL)
                                   ? high_prio_task->curr_prio
                                   : holder->base_prio);
  }
  OS_EXIT_CRITICAL();
}

os_error_t os_mutex_take(os_event_id_t id, os_size_t timeout) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  if (os_ctx.events[id].type != OS_EVENT_MUTEX) {
    OS_EXIT_CRITICAL();
    return OS_WRONG_EVENT;
  }
  os_curr_task->wait_return = OS_WAIT_RET_OK;
  os_tcb_t *holder = os_ctx.events[id].holder;
  if (holder != OS_NULL) {
    os_curr_task->state = OS_TASK_WAITING_FOR_EVENT;
    os_curr_task->delay = timeout;
    os_curr_task->wait_event = &os_ctx.events[id];
    os_queue_pop(&os_ctx.priorities[os_curr_task->curr_prio]);
    OS_PRIORITY_UNREADY(os_curr_task->curr_prio);
    if (holder->curr_prio < os_curr_task->curr_prio) {
      os_update_priority(holder, os_curr_task->curr_prio);
    }
    os_queue_push(os_curr_task, &os_ctx.events[id].queue);
    OS_EXIT_CRITICAL();
    os_schedule();
  } else {
    os_ctx.events[id].holder = os_curr_task;
    OS_EXIT_CRITICAL();
  }
  switch (os_curr_task->wait_return) {
  case OS_WAIT_RET_OK:
    return OS_OK;
  case OS_WAIT_RET_TIMEOUT:
    return OS_TIMEOUT;
  default:
    return OS_ERROR;
  }
}

os_error_t os_mutex_give(os_event_id_t id) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  if (os_ctx.events[id].type != OS_EVENT_MUTEX) {
    OS_EXIT_CRITICAL();
    return OS_WRONG_EVENT;
  }
  if (os_curr_task->curr_prio != os_curr_task->base_prio) {
    os_queue_remove(os_curr_task, &os_ctx.priorities[os_curr_task->curr_prio]);
    OS_PRIORITY_UNREADY(os_curr_task->curr_prio);
    os_curr_task->curr_prio = os_curr_task->base_prio;
    os_queue_push(os_curr_task, &os_ctx.priorities[os_curr_task->curr_prio]);
    OS_PRIORITY_READY(os_curr_task->curr_prio);
  }
  os_tcb_t *high_prio_task = _mutex_get_next(&os_ctx.events[id]);
  if (high_prio_task != OS_NULL) {
    os_ctx.events[id].holder = high_prio_task;
    os_queue_remove(high_prio_task, &os_ctx.events[id].queue);
    os_ctx.events[id].holder->state = OS_TASK_READY;
    os_ctx.events[id].holder->delay = 0;
    os_queue_push(os_ctx.events[id].holder,
                  &os_ctx.priorities[os_ctx.events[id].holder->curr_prio]);
    OS_PRIORITY_READY(os_ctx.events[id].holder->curr_prio);
    OS_EXIT_CRITICAL();
    os_schedule();
  } else {
    os_ctx.events[id].holder = OS_NULL;
    OS_EXIT_CRITICAL();
  }
  return OS_OK;
}

os_error_t os_semaphore_take(os_event_id_t id, os_size_t timeout) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  if (os_ctx.events[id].type != OS_EVENT_SEMAPHORE) {
    OS_EXIT_CRITICAL();
    return OS_WRONG_EVENT;
  }
  os_curr_task->wait_return = OS_WAIT_RET_OK;
  if (os_ctx.events[id].count == 0) {
    os_curr_task->state = OS_TASK_WAITING_FOR_EVENT;
    os_curr_task->delay = timeout;
    os_curr_task->wait_event = &os_ctx.events[id];
    os_queue_pop(&os_ctx.priorities[os_curr_task->curr_prio]);
    OS_PRIORITY_UNREADY(os_curr_task->curr_prio);
    os_queue_push(os_curr_task, &os_ctx.events[id].queue);
    OS_EXIT_CRITICAL();
    os_schedule();
  } else {
    os_ctx.events[id].count--;
    OS_EXIT_CRITICAL();
  }
  switch (os_curr_task->wait_return) {
  case OS_WAIT_RET_OK:
    return OS_OK;
  case OS_WAIT_RET_TIMEOUT:
    return OS_TIMEOUT;
  default:
    return OS_ERROR;
  }
}

os_error_t os_semaphore_give(os_event_id_t id) {
  OS_DECLARE_CRITICAL();
  OS_ENTER_CRITICAL();
  if (os_ctx.events[id].type != OS_EVENT_SEMAPHORE) {
    OS_EXIT_CRITICAL();
    return OS_WRONG_EVENT;
  }
  os_ctx.events[id].count++;
  os_tcb_t *next = os_ctx.events[id].queue.first;
  if (next != OS_NULL) {
    next->state = OS_TASK_READY;
    next->delay = 0;
    os_queue_pop(&os_ctx.events[id].queue);
    os_queue_push(next, &os_ctx.priorities[next->curr_prio]);
    OS_PRIORITY_READY(next->curr_prio);
    OS_EXIT_CRITICAL();
    os_schedule();
  } else {
    OS_EXIT_CRITICAL();
  }
  return OS_OK;
}
