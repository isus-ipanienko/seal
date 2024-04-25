#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define OS_PORT_NVIC_PENDSV_PRIO_REG *((volatile os_reg_t *)0xe000ed22)
#define OS_PORT_NVIC_PENDSV_PRIO_VAL (0xff)
#define OS_PORT_NVIC_INT_CTRL_REG *((volatile os_reg_t *)0xe000ed04)
#define OS_PORT_NVIC_PENDSVSET_BIT (1UL << 28UL)

#define OS_CTX_SWITCH() os_port_context_switch()
#define OS_CTX_SWITCH_FROM_ISR() os_port_context_switch()

/** @brief This function triggers PendSV. */
void os_port_context_switch(void);

#define OS_PORT_MAX_SYSCALL_INT_PRIORITY 4U
#define OS_PORT_NVIC_OFFSET 4U
#define OS_PORT_BASEPRI_VAL                                                    \
  (OS_PORT_MAX_SYSCALL_INT_PRIORITY << OS_PORT_NVIC_OFFSET)

#define OS_PRIORITY_READY(_priority)                                           \
  do {                                                                         \
    os_ctx.ready_priorities |= (1 << (_priority));                             \
  } while (0)
#define OS_PRIORITY_UNREADY(_priority)                                         \
  do {                                                                         \
    if (os_ctx.priorities[_priority].first == OS_NULL)                         \
      os_ctx.ready_priorities &= ~(1 << (_priority));                          \
  } while (0)
#define OS_GET_HIGHEST_PRIORITY(_priorities) (31UL - __builtin_clz(_priorities))

#ifdef __cplusplus
}
#endif
