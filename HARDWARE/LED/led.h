#ifndef __LED_H
#define __LED_H

#include "sys.h"

//LED¶Ë¿Ú¶¨Òå
#define LED1 PFout(9)	// DS1
#define LED2 PFout(10)	// DS2
#define LED3 PEout(13)	// DS3
#define LED4 PEout(14)	// DS4	

void LED_Init(void);

#endif
