#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define OS_TRUE 1U
#define OS_FALSE 0U
#define OS_NULL ((void *)0)

typedef unsigned char os_bool_t;

typedef unsigned char os_u8_t;
typedef unsigned short int os_u16_t;
typedef unsigned long int os_u32_t;
typedef unsigned long long int os_u64_t;
typedef signed char os_i8_t;
typedef signed short int os_i16_t;
typedef signed long int os_i32_t;
typedef signed long long int os_i64_t;

typedef float os_f32_t;
typedef double os_f64_t;

typedef os_u32_t os_reg_t;
typedef os_u32_t os_size_t;
typedef os_u32_t os_stack_t;

/**
 * @brief This macro converts _bytes to an amount of os_stack_t entries.
 */
#define OS_PORT_BYTES_TO_SECTORS(_bytes) (_bytes >> 2)

#define OS_DISABLE_INTERRUPTS()                                                \
  do {                                                                         \
    __asm volatile("cpsid i" ::: "memory");                                    \
  } while (0)

#define OS_ENABLE_INTERRUPTS()                                                 \
  do {                                                                         \
    __asm volatile("cpsie i" ::: "memory");                                    \
  } while (0)

/**
 * @brief Call this macro at the entry of each function that has any critical
 * sections.
 */
#define OS_DECLARE_CRITICAL() os_reg_t os_critical = 0;

/**
 * @brief   Call this macro to enter a critical section.
 * @note    It requires OS_DECLARE_CRITICAL() to be called at the entry of the
 * function.
 * @warning If pairs of OS_ENTER_CRITICAL() and OS_EXIT_CRITICAL() are not
 * balanced, a kernel panic will occur after 256 unbalanced calls.
 */
#define OS_ENTER_CRITICAL()                                                    \
  do {                                                                         \
    os_critical = os_port_enter_critical();                                    \
  } while (0)

/**
 * @brief   Call this macro to exit a critical section.
 * @note    It requires OS_DECLARE_CRITICAL() to be called at the entry of the
 * function.
 * @warning If pairs of OS_ENTER_CRITICAL() and OS_EXIT_CRITICAL() are not
 * balanced, a kernel panic will occur after 256 unbalanced calls.
 */
#define OS_EXIT_CRITICAL()                                                     \
  do {                                                                         \
    os_port_exit_critical(os_critical);                                        \
  } while (0)

/**
 * @brief This function is specific to this port of osrtos. Use
 * OS_ENTER_CRITICAL() for better portability.
 * @return os_reg_t - value of basepri upon entering the critical section
 */
os_reg_t os_port_enter_critical();

/**
 * @brief This function is specific to this port of osrtos. Use
 * OS_EXIT_CRITICAL() for better portability.
 * @param new_basepri - value of basepri to restore
 */
void os_port_exit_critical(os_reg_t new_basepri);

/**
 * @brief SysTick handler used by osrtos.
 *
 * This function needs to be called each time a systick occurs. It can be either
 * be called directly as the handler, or it can be called inside another
 * handler.
 */
void os_port_systick_handler(void);

/**
 * @brief PendSV handler used by osrtos.
 *
 * This function switches the context of the CPU. It needs to be called directly
 * as the handler, otherwise it will cause a HardFault.
 */
void os_port_pendsv_handler(void);

#ifdef __cplusplus
}
#endif
