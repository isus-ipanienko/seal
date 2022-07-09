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

#ifndef NYARTOS_PORT_H
#define NYARTOS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* ------------------------------------------------------------------------------ */
/* Types */
/* ------------------------------------------------------------------------------ */

#define NYA_TRUE     1U
#define NYA_FALSE    0U
#define NYA_NULL     ((void *)0)

typedef unsigned char            nya_bool_t;

typedef unsigned char            nya_u8_t;
typedef unsigned short int       nya_u16_t;
typedef unsigned long int        nya_u32_t;
typedef unsigned long long int   nya_u64_t;
typedef signed char              nya_i8_t;
typedef signed short int         nya_i16_t;
typedef signed long int          nya_i32_t;
typedef signed long long int     nya_i64_t;

typedef float                    nya_f32_t;
typedef double                   nya_f64_t;

typedef nya_u32_t                nya_reg_t;
typedef nya_u32_t                nya_size_t;
typedef nya_u32_t                nya_stack_t;

/**
 * @brief This macro converts _bytes to an amount of nya_stack_t entries.
 */
#define NYA_PORT_BYTES_TO_SECTORS(_bytes)    (_bytes >> 2)

/* ------------------------------------------------------------------------------ */
/* Critical Section */
/* ------------------------------------------------------------------------------ */

/**
 * @brief   This macro disables interrupts.
 */
#define NYA_DISABLE_INTERRUPTS()                 \
    do                                           \
    {                                            \
        __asm volatile ("cpsid i" ::: "memory"); \
    } while (0)

/**
 * @brief   This macro enables interrupts.
 */
#define NYA_ENABLE_INTERRUPTS()                  \
    do                                           \
    {                                            \
        __asm volatile ("cpsie i" ::: "memory"); \
    } while (0)

/**
 * @brief Call this macro at the entry of each function that has any critical sections.
 */
#define NYA_DECLARE_CRITICAL()    nya_reg_t nya_critical = 0;

/**
 * @brief   Call this macro to enter a critical section.
 * @note    It requires NYA_DECLARE_CRITICAL() to be called at the entry of the function.
 * @warning If pairs of NYA_ENTER_CRITICAL() and NYA_EXIT_CRITICAL() are not balanced,
 *          a kernel panic will occur after 256 unbalanced calls.
 */
#define NYA_ENTER_CRITICAL()                      \
    do                                            \
    {                                             \
        nya_critical = nya_port_enter_critical(); \
    } while (0)

/**
 * @brief   Call this macro to exit a critical section.
 * @note    It requires NYA_DECLARE_CRITICAL() to be called at the entry of the function.
 * @warning If pairs of NYA_ENTER_CRITICAL() and NYA_EXIT_CRITICAL() are not balanced,
 *          a kernel panic will occur after 256 unbalanced calls.
 */
#define NYA_EXIT_CRITICAL()                   \
    do                                        \
    {                                         \
        nya_port_exit_critical(nya_critical); \
    } while (0)

/**
 * @brief This function is specific to this port of nyartos. Use NYA_ENTER_CRITICAL()
 *        for better portability.
 * @return nya_reg_t - value of basepri upon entering the critical section
 */
nya_reg_t nya_port_enter_critical();

/**
 * @brief This function is specific to this port of nyartos. Use NYA_EXIT_CRITICAL()
 *        for better portability.
 * @param new_basepri - value of basepri to restore
 */
void nya_port_exit_critical(nya_reg_t new_basepri);

/* ------------------------------------------------------------------------------ */
/* ISR Handlers */
/* ------------------------------------------------------------------------------ */

/**
 * @brief SysTick handler used by nyartos.
 *
 * This function needs to be called each time a systick occurs. It can be either
 * be called directly as the handler, or it can be called inside another handler.
 */
void nya_port_systick_handler(void);

/**
 * @brief PendSV handler used by nyartos.
 *
 * This function switches the context of the CPU. It needs to be called directly
 * as the handler, otherwise it will cause a HardFault.
 */
void nya_port_pendsv_handler(void);

/* ------------------------------------------------------------------------------ */
/* */
/* ------------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef NYARTOS_PORT_H */
