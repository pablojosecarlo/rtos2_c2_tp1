/* Copyright Pablo JC Alonso Castillo 2019-2020.
 * All rights reserved.
 *
 * Fecha: 2019-X-14 convertido para RTOS 2020-III-19
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Date: 2020-III-19
 */

/*==================[inclusions]=============================================*/

#include "QueueToUART.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// sAPI
#include "sapi.h"
// otros includes
#include "qmpool2.h"
#include "c2_tp1.h"
#include "leds.h"
#include "keysMef.h"


DEBUG_PRINT_ENABLE;
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

//	void initKeysMef( int, ... ); 	//inicialización de la Keys-Mef
//	void updKeysMef( void );  		//update de la Keys-Mef

/*==================[Declaraciones de Handlers asociados al FreeRTOS]========*/

TaskHandle_t updKeysMef_Handle = NULL;
TaskHandle_t myTaskLedPeriodicoHandle = NULL;
TaskHandle_t updQueueToUARTHandle =  NULL;

/*==================[Declaraciones y variables asociadas al Objeto Led]=========*/

Led led1, led2; /* multiples instancias de Led */

/*==================[start of original code]=================================*/

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

   /* ------------- INICIALIZACIONES ------------- */

   /* Inicializar la placa */
   boardConfig();

   // Inicializar UART for debug messages
   debugPrintConfigUart( UART_USB, 115200 );
   debugPrintlnString( "RTOS2 Clase2_TP1" );

   /* Inicialización de la Keys-Mef. Inicializo las 4 teclas de la CIAA*/
   initKeysMef( 4, TEC1, TEC2, TEC3, TEC4 );

   /* Inicialización de los Leds. Inicializo dos leds de la CIAA*/
   Led_ctor(&led1, LED3 );
   Led_ctor(&led2, LED2 );

   /* Inicialización de las comunicaciones con la UART */
   initQueueToUART( 100 );

   // Led para dar señal de vida
   gpioWrite( LEDB, ON );

   // Crear tareas en freeRTOS

   //Task: Maquina de estados y servicios de las teclas
   xTaskCreate(
	  updKeysMef,
      (const char *)"updKeysMef",
      configMINIMAL_STACK_SIZE*2,
      0,
      tskIDLE_PRIORITY+1,
	  updKeysMef_Handle
   );

   //Task: servicio de parpadeo de leds
   xTaskCreate(
	  myTaskLedPeriodico,
      (const char *)"myTaskLedPeriodico",
      configMINIMAL_STACK_SIZE*2,
      0,
      tskIDLE_PRIORITY+1,
	  myTaskLedPeriodicoHandle
   );

   //Task: servicio de recepción e impresion de cola de mensajes
   xTaskCreate(
	  updQueueToUART,
      (const char *)"updQueueToUART",
      configMINIMAL_STACK_SIZE*2,
      0,
      tskIDLE_PRIORITY+1,
	  updQueueToUARTHandle
   );


   // Iniciar scheduler
   vTaskStartScheduler();

   /* ------------- REPETIR POR SIEMPRE ------------- */
   while(1) {

   }

   /* NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa no es llamado por ningun S.O. */
   return 0 ;
}

/*==================[end of file]============================================*/
