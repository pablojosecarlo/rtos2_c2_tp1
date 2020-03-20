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

#include "qmpool2.h"
#include "leds.h"

/*==================[definiciones de datos internos]=========================*/

int8_t  NUM[5] = { 0, 0, 0, 0, 0 };   //arreglo para guardar los dígitos del tiempo de pulsado de los botones

/*==================[definiciones de datos externos]=========================*/

//Definida externamente
extern QueueHandle_t myQueueCommandHandle;

extern QMPool miMemPool1;
extern uint8_t memPoolSto1[];

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

   Led_setBlink( &led1, ON);  //habilito el blinking
   Led_setBlink( &led2, ON);  //habilito el blinking
   Led_setOnOff( &led2, ! Led_getOnOff( &led1 ) );  //los pongo en estados opuestos

   // ---------- REPETIR POR SIEMPRE --------------------------

     while(TRUE) {


	   // Enciendo y apago los LEDs alternativamente
       Led_twist( &led1 );            //prendo <--> apago
       Led_twist( &led2 );            //apago  <--> prendo

       if( Led_getOnOff( &led1) ){    //Si esta prendido . . .

    	   //ptr = pvPortMalloc( 7 );
		   ptr = QMPool_get( &miMemPool1, 2 );		// 2 bloques de 5 bytes

	 	   if (ptr){

	 		    *( ( char * ) ptr + 0 ) = 'L';
				*( ( char * ) ptr + 1 ) = 'E';
				*( ( char * ) ptr + 2 ) = 'D';
				*( ( char * ) ptr + 3 ) = ' ';
				*( ( char * ) ptr + 4 ) = 'O';
				*( ( char * ) ptr + 5 ) = 'N';
				*( ( char * ) ptr + 6 ) =  0 ;

				xQueueSend( myQueueCommandHandle, &ptr,  (TickType_t) 100);

		   }else{
			   //Falla alocacion de memoria
		   }
		   //printf( "LED ON\r\n" );
	   }
	   //  Repetir por siempre, cada 2000 mSeg
	   vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
   }
};

//Task: servico de recepcion e impresion de cola de mensajes
void myTaskToTextUART( void* taskParmPtr )
{
	void * ptrR;

	while (1){
		if ( xQueueReceive( myQueueCommandHandle, &ptrR, 1000 / portTICK_RATE_MS ) ){
			printf( "Recibido: %s\n", ptrR );

			//vPortFree(ptrR);
			QMPool_put( &miMemPool1, ptrR );
		}
		else {
			//puts( "falla al recibir dato del queue" );
		}
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

