/*
 * leds.c
 *
 *  Created on: Mar 19, 2020
 *      Author: Pablo Alonso Castillo
 */
#include "sapi.h"
#include "leds.h"

void   Led_ctor( Led * const me, gpioMap_t ledADD ){
	   me->ledADD     = ledADD;
	   me->ledOnOff   = OFF;  //por defecto OFF
	   me->ledBlink   = OFF;  //por defecto OFF

	   gpioWrite(ledADD, OFF);
};

void   Led_setOnOff( Led * const me, bool_t OnOff ){
	   me->ledOnOff = OnOff;
	   gpioWrite(me->ledADD, OnOff);

};

bool_t Led_getOnOff( Led * const me ){
	   //return gpioRead( me->ledADD );
	   return me->ledOnOff ;
};

void   Led_setBlink( Led * const me, bool_t OnOff ){
	   me->ledBlink = OnOff;
};

bool_t Led_getBlink( Led * const me ){
	   return me->ledBlink;
};

void   Led_twist( Led * const me){
	if( me->ledBlink )
		Led_setOnOff( me, !(me->ledOnOff) );
};
