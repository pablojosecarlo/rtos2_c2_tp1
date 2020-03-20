/*
 * leds.h
 *
 *  Created on: Mar 19, 2020
 *      Author: pablo
 */

#ifndef EXAMPLES_C_APP_INC_LEDS_H_
#define EXAMPLES_C_APP_INC_LEDS_H_

#include "sapi.h"

/* Led's attributes... */
typedef struct {
	gpioMap_t ledADD;     /* address del Led */
	bool_t    ledOnOff;   /* ledOnOff   por defecto OFF        */
	bool_t    ledBlink;   /* ledBlink   por defecto OFF        */
} Led;

/* Led's operations (Led's interface)... */
void   Led_ctor    ( Led * const me, gpioMap_t );

void   Led_setOnOff( Led * const me, bool_t );
bool_t Led_getOnOff( Led * const me);

void   Led_setBlink( Led * const me, bool_t );
bool_t Led_getBlink( Led * const me);

void   Led_twist( Led * const me);


#endif /* EXAMPLES_C_APP_INC_LEDS_H_ */
