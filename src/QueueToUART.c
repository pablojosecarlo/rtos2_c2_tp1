/*
 * queueToUART.c
 *
 *  Created on: Mar 25, 2020
 *      Author: Pablo J.C. Alonso Castillo
 */

/*==================[inclusions]=============================================*/

#include "QueueToUART.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// sAPI
#include "sapi.h"

#include "qmpool2.h"

QueueHandle_t myQueueToUARTHandle = NULL;

/*==================[Declaraciones y variables asociadas al Quantum Leaps Mem Pool]=========*/
QMPool miMemPool2;
uint8_t memPoolSto2[50*1]; // 50 bloques de 1 byte

// Inicialización del Queue y la memória
void initQueueToUART( void ){

	 // Inicializar Pool de memoria Quantum Leaps
	 QMPool_init(&miMemPool2,
	             memPoolSto2,
	             sizeof(memPoolSto2),
	             1U);  /* bloques 1 bytes cada uno */

	 myQueueToUARTHandle = xQueueCreate( 6,  sizeof( void * ) );
}

// Update del Queue y envio al puerto serie
void updQueueToUART( void * taskParmPtr ){
	void * ptr;

	while (1){
		if ( xQueueReceive( myQueueToUARTHandle, &ptr, 1000 / portTICK_RATE_MS ) ){
			printf( "queueToUART: %s\n", ptr );

			//vPortFree(ptrR);
			QMPool_put( &miMemPool2, ptr );
		}
		else {
			//puts( "falla al recibir dato del queue" );
		}
	};
}


void sendQueueToUART( uint8_t * strPtr){
	void * ptr;

	if ( strlen0( strPtr ) > sizeof( memPoolSto2 ) ){
		//error mensaje demasiado largo
		return;
	}

	//ptr = pvPortMalloc( strlen0( strPtr ) );
	ptr = QMPool_get( &miMemPool2, strlen0( strPtr ) );  //Total de strlen0( strPtr ) bloques de 1 Byte

	if (ptr){

		copiarStrToStr( strPtr, (uint8_t *) ptr );

		xQueueSend( myQueueToUARTHandle, &ptr,  (TickType_t) 100);

   }else{
	   //Falla alocacion de memoria
   }

}


/* strlen0: retorna la longitud del string s incluido el 0 final */
static uint8_t strlen0( uint8_t * s )
{
    uint8_t n;

    for (n = 0; *s != '\0'; s++)
        n++;
    return n + 1;
}

/*Copia el strI en el strO, solo considera la longitud del inicial */
static void copiarStrToStr( uint8_t * strI,  uint8_t * strO){
	uint8_t strLng;

	strLng = strlen0( strI );  // recordar que incluye el 0 final

	for( uint8_t i = 0; i < strLng ; i++){
		*( strO + i ) =  *( strI + i );
	}


}

