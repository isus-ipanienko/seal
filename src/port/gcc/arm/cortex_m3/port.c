#include "seal_private.h"

os_stack_t *os_port_init_stack(os_task_func_t entry_func, os_stack_t *stack_ptr,
                               os_stack_t stack_size, void *param) {
  os_stack_t *ptr = &stack_ptr[stack_size];
  ptr = (os_stack_t *)((os_stack_t)(ptr)&0xfffffff8U); /* align to 8 bytes */
  *(--ptr) = (os_stack_t)0x01000000UL; /* xPSR : set thumb state */
  *(--ptr) = (os_stack_t)entry_func & (os_stack_t)0xfffffffe;   /* task entry */
  *(--ptr) = (os_stack_t)os_task_exit & (os_stack_t)0xfffffffe; /* LR */
  *(--ptr) = (os_stack_t)0x0000000CUL;                          /* R12 */
  *(--ptr) = (os_stack_t)0x00000003UL;                          /* R3 */
  *(--ptr) = (os_stack_t)0x00000002UL;                          /* R2 */
  *(--ptr) = (os_stack_t)0x00000001UL;                          /* R1 */
  *(--ptr) = (os_stack_t)param;                                 /* R0 */
  *(--ptr) = (os_stack_t)0x0000000BUL;                          /* R11 */
  *(--ptr) = (os_stack_t)0x0000000AUL;                          /* R10 */
  *(--ptr) = (os_stack_t)0x00000009UL;                          /* R9 */
  *(--ptr) = (os_stack_t)0x00000008UL;                          /* R8 */
  *(--ptr) = (os_stack_t)0x00000007UL;                          /* R7 */
  *(--ptr) = (os_stack_t)0x00000006UL;                          /* R6 */
  *(--ptr) = (os_stack_t)0x00000005UL;                          /* R5 */
  *(--ptr) = (os_stack_t)0x00000004UL;                          /* R4 */
  os_stack_t *ret = ptr;
  while (ptr-- != stack_ptr) {
    *ptr = 0xdeadbeef;
  }
  return ret;
}

static os_stack_t os_exception_stack[256];

void os_port_startup(void) {
  OS_DISABLE_INTERRUPTS();

  os_curr_task = os_next_task;
  OS_PORT_NVIC_PENDSV_PRIO_REG = OS_PORT_NVIC_PENDSV_PRIO_VAL;
  os_ctx.is_running = OS_TRUE;

  /* get top of stack and align to 8 bytes */
  os_stack_t *exception_stack;
  exception_stack = &os_exception_stack[OS_ARRAY_SIZE(os_exception_stack)];
  exception_stack = (os_stack_t *)((os_stack_t)exception_stack & 0xfffffff8U);

  __asm volatile(
      "mov r0, %0                   \n" /* load exception stack as msp */
      "msr msp, r0                  \n"
      "                             \n"
      "mov r2, %1                   \n" /* load address of the current task */
      "ldr r3, [r2]                 \n" /* load new process stack pointer */
      "msr psp, r3                  \n" /* load it to psp */
      "                             \n"
      "mrs r0, control              \n" /* set psp as the current stack pointer
                                         */
      "orr r0, r0, #2               \n"
      "msr control, r0              \n"
      "isb                          \n"
      "                             \n"
      "ldmia sp!, {r4-r11}          \n" /* restore the remaining registers */
      "ldmia sp!, {r0-r3, r12, r14} \n"
      "ldmia sp!, {r1, r2}          \n"
      "orr r1, #1                   \n" /* ensure r1[0] is set */
      "                             \n"
      "cpsie i                      \n" /* enable interrupts */
      "bx r1                        \n" /* start task */
      ".align 4                     \n"
      :
      : "r"(exception_stack), "r"(os_curr_task)
      : "memory");
}

void os_port_systick_handler(void) {
  os_enter_isr();
  os_systick();
  os_exit_isr();
}
