#ifndef __CHILD_SAFETY_LOCK_H
#define __CHILD_SAFETY_LOCK_H

#include <stdbool.h>

extern volatile bool lock;

void LockUp(void);
void UnLock(void);
void LockUpdate(void);

#endif
