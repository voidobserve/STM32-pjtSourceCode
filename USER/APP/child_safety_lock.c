#include "child_safety_lock.h"
#include <stdbool.h>
#include "usart2.h"

volatile bool lock = false; // ¶ùÍ¯Ëø£¬false--¹Ø±Õ£¬true--¿ªÆô

void LockUp(void)
{
    lock = true;
}

void UnLock(void)
{
    lock = false;
}

void LockUpdate(void)
{
    if (true == lock)
    {
        LockUp();
    }
    else
    {
        UnLock();
    }
}
