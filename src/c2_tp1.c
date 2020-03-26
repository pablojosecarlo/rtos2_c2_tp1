/*
 * tareas.c
 *
 *  Created on: Mar 19, 2020
 *      Author: pablo
 */

/*==================[inclusiones]============================================*/

// Includes de FreeRTOS
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// sAPI
#include "sapi.h"
// otros includes
#include "c2_tp1.h"

#include "QueueToUART.h"

#include "leds.h"


/*==================[definiciones de datos internos]=========================*/

int8_t  NUM[5] = { 0, 0, 0, 0, 0 };   //arreglo para guardar los dígitos del tiempo de pulsado de los botones

/*==================[definiciones de datos externos]=========================*/

//Definida externamente
extern QueueHandle_t myQueueCommandHandle;

extern Led led1, led2;

/*==========================[funciones]======================================*/

//Task: servicio de parpadeo de leds
//Enciende alternativamente led1 y led2
void myTaskLedPeriodico( void* taskParmPtr )
{
   // Tarea periodica cada 2000 ms
   portTickType xPeriodicity =  2000 / portTICK_RATE_MS;
   portTickType xLastWakeTime = xTaskGetTickCount();

   void *ptr = NULL;
   void *ptr2 = NULL;
   uint8_t elMsj[7];

   Led_setBlink( &led1, ON);  //habilito el blinking
   Led_setBlink( &led2, ON);  //habilito el blinking
   Led_setOnOff( &led2, ! Led_getOnOff( &led1 ) );  //los pongo en estados opuestos

   // ---------- REPETIR POR SIEMPRE --------------------------

     while(TRUE) {


	   // Enciendo y apago los LEDs alternativamente
       Led_twist( &led1 );            //prendo <--> apago
       Led_twist( &led2 );            //apago  <--> prendo

       if( Led_getOnOff( &led1) ){    //Si esta prendido . . .

		   elMsj[ 0 ] = 'l';
		   elMsj[ 1 ] = 'e';
		   elMsj[ 2 ] = 'd';
		   elMsj[ 3 ] = ' ';
		   elMsj[ 4 ] = 'o';
		   elMsj[ 5 ] = 'n';
		   elMsj[ 6 ] =  0 ;

		   sendQueueToUART( elMsj );

		   //printf( "LED ON\r\n" );
	   }
	   //  Repetir por siempre, cada 2000 mSeg
	   vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
   }
};


//Separa los digitos de un uint16_t y los guarda en el arreglo publico NUM[]
void uint16ToAscii (uint16_t valor)
{
	union
	{
		uint8_t sector[1];
		uint16_t x;
	} unionX;

	unionX.x = valor;
	BIN16_A_DIGITAL5(unionX.sector[1], unionX.sector[0]);	//CONVIERTE HADD Y LADD A NUM[4] -> NUM[0]
}

//Toma dos bites que forman un numero de 16 bits y lo convierte en digitos
//que guarda en el arreglo publico NUM[]
void BIN16_A_DIGITAL5(uint8_t HADD, uint8_t LADD){

//convierte un uint_16 formando por dos mitades uint8_t en dígitos ascii que guarda en el arreglo NUM[5]

	NUM[0] = 0; NUM[1] = 0; NUM[2] = 0; NUM[3] = 0; NUM[4] = 0;

	//BLOQUE DE PASAR DE BINARIO A GRUPOS DE UNIDADES, DECENAS, CENTENAS, MILES. ETC
	if (LADD & 0b00000001){ NUM[0] +=1; }//1
	if (LADD & 0b00000010){ NUM[0] +=2; }//2
	if (LADD & 0b00000100){ NUM[0] +=4; }//4
	if (LADD & 0b00001000){ NUM[0] +=8; }//8
	if (LADD & 0b00010000){ NUM[0] +=6; NUM[1] += 1; }//16
	if (LADD & 0b00100000){ NUM[0] +=2; NUM[1] += 3; }//32
	if (LADD & 0b01000000){ NUM[0] +=4; NUM[1] += 6; }//64
	if (LADD & 0b10000000){ NUM[0] +=8; NUM[1] += 2; NUM[2] += 1; }//128

	if (HADD & 0b00000001){ NUM[0] +=6; NUM[1] += 5; NUM[2] += 2; }//256
	if (HADD & 0b00000010){ NUM[0] +=2; NUM[1] += 1; NUM[2] += 5; }//512
	if (HADD & 0b00000100){ NUM[0] +=4; NUM[1] += 2; NUM[2] += 0; NUM[3] += 1; }//1024
	if (HADD & 0b00001000){ NUM[0] +=8; NUM[1] += 4; NUM[2] += 0; NUM[3] += 2; }//2048
	if (HADD & 0b00010000){ NUM[0] +=6; NUM[1] += 9; NUM[2] += 0; NUM[3] += 4; }//4096
	if (HADD & 0b00100000){ NUM[0] +=2; NUM[1] += 9; NUM[2] += 1; NUM[3] += 8; }//8192
	if (HADD & 0b01000000){ NUM[0] +=4; NUM[1] += 8; NUM[2] += 3; NUM[3] += 6; NUM[4] += 1; }//16384
	if (HADD & 0b10000000){ NUM[0] +=8; NUM[1] += 6; NUM[2] += 7; NUM[3] += 2; NUM[4] += 3; }//32768
	//BLOQUE DE PASAR DE GRUPOS DE UNIDADES, DECENAS, CENTENAS Y MILES
	//A UNIDADES DE: UNIDAD, DECENA, CENTENA, MIL, DIEZ MIL, ETC.
	while(NUM[0] >= 0) { NUM[0] -= 10; if ( NUM[0] >= 0 ) NUM[1] += 1;} NUM[0] += 10;
	while(NUM[1] >= 0) { NUM[1] -= 10; if ( NUM[1] >= 0 ) NUM[2] += 1;} NUM[1] += 10;
	while(NUM[2] >= 0) { NUM[2] -= 10; if ( NUM[2] >= 0 ) NUM[3] += 1;} NUM[2] += 10;
	while(NUM[3] >= 0) { NUM[3] -= 10; if ( NUM[3] >= 0 ) NUM[4] += 1;} NUM[3] += 10;

}

