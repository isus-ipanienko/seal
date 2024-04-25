#include "seal.h"

/**
 * @brief       This is the system idle task.
 * @warning     This task needs to be ready to run at all times.
 */
void idle_entry(void *param) {
  OS_UNUSED(param);
  while (1) {
  }
}
