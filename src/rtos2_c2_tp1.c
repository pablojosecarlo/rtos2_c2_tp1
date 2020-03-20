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
 * Date: 2019-X-14
 */

/*==================[inclusions]=============================================*/

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// sAPI
#include "sapi.h"
// otros includes
#include "qmpool_Original.h"
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

/*==================[start of original code]=================================*/

TaskHandle_t updKeysMef_Handle = NULL;
TaskHandle_t myTaskLedPeriodicoHandle = NULL;
TaskHandle_t myTaskToTextUARTHandle = NULL;

QueueHandle_t myQueueCommandHandle = NULL;


/*==================[Declaraciones y variables asociadas al Quantum Leaps]=========*/
QMPool miMemPool1;
uint8_t memPoolSto1[10*40]; // 10 bloques de 40 bytes

/*==================[Declaraciones y variables asociadas al Objeto Led]=========*/
Led led1, led2; /* multiples instancias de Led */

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){

   /* ------------- INICIALIZACIONES ------------- */

   /* Inicializar la placa */
   boardConfig();
   // Pool de memoria Quantum Leaps

   QMPool_init(&miMemPool1,
               memPoolSto1,
               sizeof(memPoolSto1),
               5U);  /* bloques 5 bytes cada uno */

   // UART for debug messages
   debugPrintConfigUart( UART_USB, 115200 );
   debugPrintlnString( "RTOS2 Clase2_TP1" );

   /* Inicializacion de la Keys-Mef */
   initKeysMef( 4, TEC1, TEC2, TEC3, TEC4 );

   /* Inicializacion de los Leds    */
   Led_ctor(&led1, LED3 );
   Led_ctor(&led2, LED2 );

   // Led para dar señal de vida
   gpioWrite( LEDB, ON );

   // Crear tareas en freeRTOS

   xTaskCreate(
	  updKeysMef,                 // Funcion de la tarea a ejecutar
      (const char *)"updKeysMef", // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
	  updKeysMef_Handle           // Puntero a la tarea creada en el sistema
   );

   xTaskCreate(
	  myTaskLedPeriodico,         // Funcion de la tarea a ejecutar
      (const char *)"myTaskLedPeriodico",    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
	  myTaskLedPeriodicoHandle    // Puntero a la tarea creada en el sistema
   );

   xTaskCreate(
	  myTaskToTextUART,         // Funcion de la tarea a ejecutar
      (const char *)"myTaskToTextUART",    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
	  myTaskToTextUARTHandle      // Puntero a la tarea creada en el sistema
   );

   myQueueCommandHandle = xQueueCreate( 6,  sizeof( void * ) );

   // Iniciar scheduler
   vTaskStartScheduler();

   /* ------------- REPETIR POR SIEMPRE ------------- */
   while(1) {

   }

   /* NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa no es llamado por ningun S.O. */
   return 0 ;
}

/*==================[end of file]============================================*/
