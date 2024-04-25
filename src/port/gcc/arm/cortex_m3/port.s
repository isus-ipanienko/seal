.extern  os_curr_task
.extern  os_next_task

.global  os_port_pendsv_handler
.global  os_port_context_switch
.global  os_port_enter_critical
.global  os_port_exit_critical

.equ OS_PORT_NVIC_INT_CTRL_REG,     0xE000ED04
.equ OS_PORT_NVIC_PENDSVSET_BIT,    0x10000000
.equ OS_PORT_BASEPRI_VAL,           0x40

.text
.align 4
.thumb
.syntax unified

.thumb_func
os_port_context_switch:
    ldr r0, =OS_PORT_NVIC_INT_CTRL_REG
    ldr r1, =OS_PORT_NVIC_PENDSVSET_BIT
    str r1, [r0]
    dsb
    isb
    bx lr

.thumb_func
os_port_enter_critical:
    cpsid i
    push {r1}
    ldr r0, =OS_PORT_BASEPRI_VAL
    mrs r1, basepri
    msr basepri, r0
    dsb
    isb
    mov r0, r1
    pop {r1}
    cpsie i
    bx lr

.thumb_func
os_port_exit_critical:
    cpsid i
    msr basepri, r0
    dsb
    isb
    cpsie i
    bx lr

.thumb_func
os_port_pendsv_handler:
    cpsid i                  /* disable interrupts */
    ldr r0, =OS_PORT_BASEPRI_VAL
    msr basepri, r0
    dsb
    isb
    cpsie i

    mrs r0, psp              /* load old process stack pointer */
    stmdb r0!, {r4-r11}      /* push registers */

    ldr r1, =os_curr_task    /* get address of current taskptr */
    ldr r2, [r1]             /* get address of current task and stack pointer */
    str r0, [r2]             /* store stack pointer to current task */

    ldr r3, =os_next_task    /* get address of next taskptr */
    ldr r2, [r3]             /* get address of next task and stack pointer */
    str r2, [r1]             /* set next task as current task */

    ldr r0, [r2]             /* get value of next stack pointer */
    ldmia r0!, {r4-r11}      /* pop registers */
    msr psp, r0              /* load new process stack pointer */

    cpsid i                  /* enable interrupts */
    mov r0, #0
    msr basepri, r0
    dsb
    isb
    cpsie i
    bx lr

.end
