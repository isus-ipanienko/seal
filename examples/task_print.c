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
#include "nyartos.h"
#include "main.h"

void print_entry(void *param)
{
    NYA_UNUSED(param);

    nya_sleep(2000);

    if (nya_semaphore_take(NYA_SEMAPHORE_ID_FOO,
                           2000) == NYA_TIMEOUT)
    {
        while (1)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)"semaphore timeout\n",
                              sizeof("semaphore timeout\n") - 1,
                              100);
            nya_sleep(250);
        }
    }

    if (nya_mutex_take(NYA_MUTEX_ID_FOO,
                       2000) == NYA_TIMEOUT)
    {
        while (1)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)"mutex timeout\n",
                              sizeof("mutex timeout\n") - 1,
                              100);
            nya_sleep(250);
        }
    }

    while (1)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"foobar\n",
                          sizeof("foobar\n") - 1,
                          100);
        nya_sleep(250);
    }
}
