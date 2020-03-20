/* Copyright Pablo JC Alonso Castillo 2019.
 * All rights reserved.
 *
 * Examen final PdM 10° Cohorte 2019
 *
 * Fecha: 2019-X-14
 */

/*
 * Librería para el uso de botones/pulsadores/teclas caso "casi" general
 *
 * Tomando como datos de entrada en la inicialización de la MEF las direcciones de los GPIO, genera un arreglo de estados y eventos que
 * actualiza en el update de la MEF  y luego procesa en acciones, en el servicio de la MEF
 *
 * Se debe indicar en el define de KEYSMAX el número de teclas a implementar
 *
 * Admite multiples teclas activadas simultáneamente.
 *
 * Utiliza una MEF implementando:
 * 		Estados
 * 		Transiciones
 * 		Eventos
 *
 *  Más detalles en keysMef.c
 *
 * 	Detecta
 * 	los estados:
 * 				KEY_ARR_LONG	-> ESTADO PERMANENTE INICIAL    -> BOTÓN ARRIBA (SIN PULSARSE)
 * 				KEY_FALL		-> ESTADO DE TRANSICIÓN 		-> BOTÓN PULSANDOSE (ARRIBA -> ABAJO)
 * 				KEY_ABA_SHORT  	-> ESTADO DE TRANSICIÓN 		-> BOTÓN ABAJO CORTO TIEMPO
 * 				KEY_ABA_TRANS 	-> ESTADO DE TRANSICIÓN 		-> BOTÓN ABAJO (CORTO -> LARGO)
 * 				KEY_ABA_LONG	-> ESTADO PERMANENTE FINAL    	-> BOTÓN ABAJO (PULSADO PERMANENTEMENTE)
 * 				KEY_RISE      	-> ESTADO DE TRANSICIÓN 		-> BOTÓN PULSANDOSE (ABAJO -> ARRIBA)
 * 				KEY_ARR_SHORT 	-> ESTADO DE TRANSICIÓN 		-> BOTÓN ARRIBA CORTO TIEMPO+ KEY_TOP 	- LATENCIA DEL DOBLE CLICK
 * 				KEY_ARR_TRANS 	-> ESTADO DE TRANSICIÓN 		-> BOTÓN ARRIBA (CORTO -> LARGO)		- LATENCIA DEL DOBLE CLICK
 *  Las transiciones:
 *				TRANS_KEY_TOP  	-> TRANSICIÓN QUE OCURRE CUANDO EL UPDATE DE LA MEF LEE EL PULSADOR Y LO DETECTA EN ALTO
 *				TRANS_KEY_TOP  	-> TRANSICIÓN QUE OCURRE CUANDO EL UPDATE DE LA MEF LEE EL PULSADOR Y LO DETECTA EN BAJO
 *	Los Eventos:
 *				EVENT_KEY_UP	-> EVENTO TECLA SUBIENDO
 *				EVENT_KEY_DOWN	-> EVENTO TECLA BAJANDO
 *				EVENT_KEY_CLICK	-> EVENTO TECLA PULSADA Y SOLTADA RÁPIDO (KEY PRESS)
 *				EVENT_KEY_2CLICK-> EVENTO TECLA PULSADA Y SOLTADA DOS VECES RÁPIDO (DOBLE CLICK)
 *				EVENT_KEY_NULL	-> EVENTO NULO (NINGÚN EVENTO)
 *
 * Se inicializa con la función:
 *
 * 				void initKeysMef( int, ... )
 *
 * 				Los argumentos de esta función son los GPIO donde se conectan las teclas/botones/pulsadores
 * 				se supone que son OC cargados con resistencias y que no pulsados se encuentran el alto.
 *
 * Se hace el update con la función:
 *
 * 				void updKeysMef( void )
 *
 * 				En el uddate se detectan las transiciones y en función de ellas y los estados previos se
 * 				determinan los estados futuros y los eventos, actualizándose el arreglo de teclas
 *
 * Se hace el servicio con la función:
 *
 *				void srvKeyMef( void (*f)( *void ), ... ) PERO. . . para no complicarla uso
 *
 *				void srvKeysMef( void )
 *
 *				En el servicio se camina el arreglo de teclas y se actua en concordancia a los estados
 *				y eventos de interés.
 *				En nuestro caso mando a la salida serie casi todos los estados y eventos indicando
 *				índice de tecla y puerto GPIO
 *
 *	NOTA:
 *	Los servicios de los estados
 *
 * 		KEY_FALL		-> 50  mS
 * 		KEY_RISE      	-> 50  mS
 * 		KEY_ABA_TRANS 	-> 800 mS
 * 		KEY_ARR_TRANS 	-> 800 mS
 *
 * 	tienen tiempos asociados con la estabilizacion del pulsador: KEY_FALL-> 50 mS y KEY_RISE -> 50 mS
 *
 * 	Con la latencia del evento doble click EVENT_KEY_2CLICK: KEY_ABA_TRANS-> 800 mS i.e. el intervalo
 * 	durante el cual puedo volver a pulsar la tecla para generar el evento
 *
 * 	y con la latencia del que sería el evento complementario EVENT_KEY_CLICK: KEY_ARR_TRANS-> 800 mS
 * 	i.e. el intervalo durante el cual puedo liberar la tecla pulsada para generar el evento
 *
 * 	NOTA: El evento EVENT_KEY_2CLICK exige pulsada + soltada + pulsada porque se inicia con la tecla
 * 	      arriba y hay que llevarla dos veces abajo.
 *
 * 	      El evento EVENT_KEY_CLICK exige soltada + pulsada + soltada PERO como se inicia con la tecla
 * 	      arriba, eso ya cuenta como la primer soltada y solo resta pulsar y soltar a tiempo
 *
 *  Pensaba implementar una librería para leer un encoder de 12 bits y un display i2c lcd, pero. . . .
 *  ¿Cómo podrian uds verificar que todo anda sin el i2c lcd (que es un desarrollo mio con el pic 16F819
 *  y sin el encoder tampoco?. Así que lo dejo como está.
 *
 */



/*=======================[Evitar inclusion multiple comienzo]==================================*/

#ifndef _KEYSMEF_H_
#define _KEYSMEF_H_

/*=======================[Inclusiones de dependencias de funciones publicas]===================*/

#include "sapi.h"
#include "stdarg.h"

/*=======================[C++ comienzo]========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=======================[Macros de definicion de constantes publicas]=========================*/

#define KEYSMAX 20	     //número de teclas a usar por defecto

/*=======================[Macros estilo funcion publicas]======================================*/

//#define DEBUG

#ifdef DEBUG
	# define PRINT_DEBUG(x) printf x
    # define LED_DEBUG(x)   gpioWrite x
#else
	# define PRINT_DEBUG(x) do {} while (0)
	# define LED_DEBUG(x)   do {} while (0)
#endif


/*=======================[Definiciones de tipos de datos publicos]=============================*/

/*=======================[Tipo de datos enumerado]=============================================*/

//Estados
typedef enum { KEY_FALL,        //TECLA BAJANDO  TRANSITORIO ARRIBA -> ABAJO
			   KEY_RISE,        //TECLA SUBIENDO TRANSITORIO ABAJO  -> ARRIBA

			   KEY_ARR_SHORT,	//TECLA ARRIBA CORTO TIEMPO
			   KEY_ARR_TRANS,	//TECLA ARRIBA TRANSITORIO CORTO -> LARGO
			   KEY_ARR_LONG,	//TECLA ARRIBA LARGO TIEMPO

			   KEY_ABA_SHORT,	//TECLA ABAJO CORTO TIEMPO
			   KEY_ABA_TRANS,	//TECLA ABAJO TRANSITORIO CORTO -> LARGO
			   KEY_ABA_LONG		//TECLA ABAJO LARGO TIEMPO
} keyState_t;

//Transiciones
typedef enum { TRANS_KEY_TOP ,		//TECLA PULSANDO ARRIBA
	   	   	   TRANS_KEY_BOTT		//TECLA PULSANDO ABAJO
} keyTrans_t;

//Eventos
typedef enum { EVENT_KEY_UP,		//EVENTO TECLA SUBIENDO
			   EVENT_KEY_DOWN,		//EVENTO TECLA BAJANDO
			   EVENT_KEY_CLICK,		//EVENTO TECLA PULSADA Y SOLTADA (aka: EVENT_KEY_PRESS)
			   EVENT_KEY_2CLICK,	//EVENTO TECLA PULSADA Y SOLTADA DOS VECES RÁPIDO
			   EVENT_KEY_NULL		//EVENTO NULO (NINGÚN EVENTO)
} keyEvent_t;

//Atributos de la tecla
typedef struct {
	gpioMap_t  tecla;
	keyState_t estado;
	keyEvent_t evento;
	uint32_t   tiempoDOWN;
} keyBrd_t;

/*=======================[Macros de definicion de constantes publicas]=========================*/

/*=======================[Prototipos de funciones publicas]====================================*/

//inicialización de la Keys-Mef
void initKeysMef( int, ... );

//update de la Keys-Mef
void updKeysMef( void* );

//servicio de la Keys-Mef

//caso general. . .
//void srvKeyMef( void (*f)( *void ), ... );

//Nuestro caso particular . . la llamo desde updKeysMef
static void srvKeysMef( void );


/*=======================[C++ fin]=============================================================*/

#ifdef __cplusplus
}
#endif

/*=======================[Evitar inclusion multiple fin]=======================================*/

#endif /* _KEYSMEF_ */


/*Fin*/
