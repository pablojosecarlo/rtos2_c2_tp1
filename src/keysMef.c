/* Copyright Pablo JC Alonso Castillo 2019.
 * All rights reserved.
 *
 *
 * Fecha: 2019-X-14
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
#include "keysMef.h"

// Definidas externamente
extern QueueHandle_t myQueueCommandHandle;
extern QMPool miMemPool1;
extern uint8_t memPoolSto1[];
extern int8_t  NUM[5];

// Unidades de Tiempo Mínimo para estados y eventos
const portTickType xTiempoMinFall    =  50  / portTICK_RATE_MS; //mSeg para pasar de arriba a abajo short
const portTickType xTiempoMinRise    =  50  / portTICK_RATE_MS; //mSeg para pasar de abajo a arriba short + evento click
const portTickType xTiempoMinDown    =  500 / portTICK_RATE_MS; //mSeg para pasar de abajo  short a abajo  long
const portTickType xTiempoMinUp      =  500 / portTICK_RATE_MS; //mSeg para pasar de arriba short a arriba long + evento 2click

// El tiempo antirrebote debe ser el menor de todos
const portTickType xTiempoAntiRebote =  25  / portTICK_RATE_MS; //mSeg antirrebote

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

static int nKEYS = KEYSMAX;			//número de teclas a usar por defecto
static keyBrd_t teclas[ KEYSMAX ];  //arreglo de puertos (teclas), estados y eventos


/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

/*==================[start of original code]=================================*/

/*------------------------------------------------------------------------------------------------------------------------------------------------------------- */
/* void initKeysMef( uint8_t, ...  )
 * Setea el estado incial del arreglo de todas las teclas y los puertos asociados
 * el arreglo contiene:  índice, puerto (tecla), estado y evento
 * Argumentos: GPIO's
 * salida: void
 /*------------------------------------------------------------------------------------------------------------------------------------------------------------- */
void initKeysMef( int nArgs, ... ){

	if ( nArgs > KEYSMAX ) nArgs = KEYSMAX;  //APB sencillo, podria complicarlo un poco pero ahora no. . .

	va_list ap;

	va_start(ap, nArgs);

	for(uint8_t i = 0; i < nArgs; i++){
		teclas[i].tecla =  va_arg(ap, int); 		//Si quiero castear a gpioMap_t lo permite revirtiendo a int y luego se cuelga :-(
		gpioConfig( teclas[i].tecla, GPIO_INPUT );  //supongo que podria ser también GPIO_INPUT_PULLUP. . . hay que ver que puertos lo toleran. . .
		teclas[i].estado     = KEY_ARR_LONG;		//También podría en teoría ser KEY_ABA_LONG. . .
		teclas[i].evento     = EVENT_KEY_NULL;		//No hay eventos al inicializar
		teclas[i].tiempoDOWN = 0;					//No hay tiempo abajo de la key
	}

	va_end(ap);

	nKEYS = nArgs;	//Recupero el verdadero número de teclas a loopear.
}

/*------------------------------------------------------------------------------------------------------------------------------------------------------------- */
/* void updKeysMef()
 *
 * update de la Máquina de estados finitos de las teclas
 * escanea el arreglo teclas[] actualizando el estado de cada una según:
 *
 * estado previo + estado actual   	= siguiente estado
 * prevState     + actualState     	= nextState
 *
 * KEY_ARR_LONG  + TRANS_KEY_TOP   	= KEY_ARR_LONG		-> ESTADO PERMANENTE INICIAL    -> BOTÓN ARRIBA (SIN PULSARSE)
 * KEY_ARR_LONG  + TRANS_KEY_BOTT  	= KEY_FALL			-> TRANSICIÓN (ARRIBA->ABAJO)   -> BOTÓN PULSANDOSE - BOTON ARRIBA LARGO (FIN)
 *
 * KEY_FALL      + TRANS_KEY_TOP   	= KEY_ARR_SHORT(*)	-> INESTABLE - (se debe aumentar delay de KEY_FALL)
 * KEY_FALL      + TRANS_KEY_BOTT  	= KEY_ABA_SHORT		-> ESTADO BOTÓN ABAJO CORTO (INICIO)       					-> EVENTO KEY_DOWN
 *
 * KEY_ABA_SHORT + TRANS_KEY_TOP    = KEY_RISE			-> TRANSICIÓN (ABAJO->ARRIBA) BOTÓN ABAJO CORTO (FIN)		-> EVENTO KEY_CLICK (INICIO 1 NO SE USA)
 * KEY_ABA_SHORT + TRANS_KEY_BOTT   = KEY_ABA_TRANS		-> TRANSICIÓN BOTÓN ABAJO (CORTO->LARGO)				   	-> LATENCIA DEL EVENTO KEY_CLICK
 *
 * KEY_ABA_TRANS + TRANS_KEY_TOP    = KEY_RISE			-> TRANSICIÓN (ABAJO->ARRIBA)								-> EVENTO KEY_CLICK (INICIO 2)
 * KEY_ABA_TRANS + TRANS_KEY_BOTT   = KEY_ABA_LONG		-> ESTADO BOTÓN ABAJO LARGO (INICIO)
 *
 * KEY_ABA_LONG  + TRANS_KEY_TOP    = KEY_RISE			-> TRANSICIÓN (ABAJO->ARRIBA) 	-> BOTÓN SOLTÁNDOSE - BOTÓN ABAJO LARGO (FIN)
 * KEY_ABA_LONG  + TRANS_KEY_BOTT   = KEY_ABA_LONG		-> ESTADO PERMANENTE 			-> BOTÓN ABAJO (SIEMPRE PULSADO)
 *
 * KEY_RISE      + TRANS_KEY_TOP    = KEY_ARR_SHORT		-> BOTÓN ARRIBA CORTO (INICIO)								-> EVENTO KEY_UP
 * KEY_RISE      + TRANS_KEY_BOTT   = KEY_ABA_SHORT(*)	-> INESTABLE - (de debe aumentar delay de KEY_RISE)
 *
 * KEY_ARR_SHORT + TRANS_KEY_TOP	= KEY_ARR_TRANS		-> TRANSICIÓN BOTÓN ARRIBA (CORTO->LARGO) 					-> LATENCIA DEL DOBLE CLICK
 * KEY_ARR_SHORT + TRANS_KEY_BOTT	= KEY_FALL			-> TRANSICIÓN (ARRIBA->ABAJO) BOTÓN ARRIBA CORTO (FIN) 		-> EVENTO DOBLE CLICK (INICIO 1 NO SE USA)
 *
 * KEY_ARR_TRANS + TRANS_KEY_TOP	= KEY_ARR_LONG		-> ESTADO PERMANENTE - BOTÓN ARRIBA (RETORNO ESTADO PERMANENTE INICIAL)
 * KEY_ARR_TRANS + TRANS_KEY_BOTT	= KEY_FALL			-> TRANSICIÓN (ARRIBA->ABAJO) 								-> EVENTO DOBLE CLICK (INICIO 2)(+)
 *
 *    Recordar que TRANS_KEY_TOP y TRANS_KEY_BOTT son transiciones que se detectan en cada loop del updKeysMef
 *
 *(*) Estados posibles pero sub transitorios se corrige aumentando los delays de KEY_RISE y KEY_FALL
 *    desgraciadamente no hay un arreglo satisfactorio de siguiente estado a menos que se distingan
 *    entre los KEY_RISE y KEY_FALL de los correspondientes estados _SHORT Y _LONG. Una complicación
 *    que dejaré para la próxima. . . . ;-)
 *
 *(+) AL evento doble click ( EVENT_KEY_2CLICK ) -como a todo doble click- hay que agarrale la mano con el timming, pero anda Joya!  ;-)
 */
/*------------------------------------------------------------------------------------------------------------------------------------------------------------- */


void updKeysMef( void* taskParmPtr ){

	keyState_t prevState;   //Estado previo
	keyState_t nextState;	//Estado siguiente
	keyTrans_t transState;	//Transicion
	keyEvent_t nextEvent;	//Evento

    // ---------- REPETIR POR SIEMPRE --------------------------

    while(TRUE) {

 	    // El antirrebote. . .
 	    vTaskDelay( xTiempoAntiRebote );

		for (uint8_t i = 0; i < nKEYS; i++) {

			//El estado previo:
			prevState = teclas[i].estado;

			//La transición entre estados:
			//         teclas con pull-ups => pulsada    = TRANS_KEY_BOTT,
			//          					  no pulsada = TRANS_KEY_TOP
			if ( gpioRead( teclas[i].tecla ) ) {
				transState = TRANS_KEY_TOP;
			} else {
				transState = TRANS_KEY_BOTT;
			};

			//El siguiente evento es por defecto nulo (hasta que se demuestre lo contrario!!)
			nextEvent = EVENT_KEY_NULL;

			//El siguiente estado
			switch ( prevState ){
				case KEY_ARR_LONG:
					switch ( transState ){
					  case TRANS_KEY_TOP : nextState = KEY_ARR_LONG;   break;
					  case TRANS_KEY_BOTT: nextState = KEY_FALL;       break;
				} break;
				case KEY_FALL:
					  switch ( transState ){
					  case TRANS_KEY_TOP : nextState = KEY_ARR_SHORT;  break;
					  case TRANS_KEY_BOTT: nextState = KEY_ABA_SHORT;
										   nextEvent = EVENT_KEY_DOWN; break;	//EVENT_KEY_DOWN
				} break;
				case KEY_ABA_SHORT:
					  switch ( transState ){
					  case TRANS_KEY_TOP : nextState = KEY_RISE;       break;
					  case TRANS_KEY_BOTT: nextState = KEY_ABA_TRANS;  break;
				} break;
				case KEY_ABA_TRANS:
					  switch ( transState ){
					  case TRANS_KEY_TOP : nextState = KEY_RISE;
										   nextEvent = EVENT_KEY_CLICK;break;	//EVENT_KEY_CLICK
					  case TRANS_KEY_BOTT: nextState = KEY_ABA_LONG;   break;
				} break;
				case KEY_ABA_LONG:
					  switch ( transState ){
					  case TRANS_KEY_TOP : nextState = KEY_RISE;       break;
					  case TRANS_KEY_BOTT: nextState = KEY_ABA_LONG;   break;   //permanece pulsada abajo
				} break;
				case KEY_RISE:
					  switch ( transState ){
					  case TRANS_KEY_TOP : nextState = KEY_ARR_SHORT;
										   nextEvent = EVENT_KEY_UP;   break;	//EVENT_KEY_UP
					  case TRANS_KEY_BOTT: nextState = KEY_ABA_SHORT;  break;
				} break;
				case KEY_ARR_SHORT:
					  switch ( transState ){
					  case TRANS_KEY_TOP : nextState = KEY_ARR_TRANS;  break;
					  case TRANS_KEY_BOTT: nextState = KEY_FALL;       break;
				} break;
				case KEY_ARR_TRANS:
					  switch ( transState ){
					  case TRANS_KEY_TOP : nextState = KEY_ARR_LONG;    break;
					  case TRANS_KEY_BOTT: nextState = KEY_FALL;
										   nextEvent = EVENT_KEY_2CLICK;break;	//EVENT_KEY_2CLICK
				} break;
			}
			teclas[i].estado = nextState;
			teclas[i].evento = nextEvent;
		}

		//llamar al servicio de keys para ver que hacer con los estados, eventos y tiempos
		srvKeysMef();

    }
}

/*------------------------------------------------------------------------------------------------------------- */
// void srvKeysMef( void (*f)( *void ), ... );
//
// servicio de la Maquina de estados finitos de las teclas escanea el arreglo teclas[] y actua según el estado.
//
// Para no complicarla, pongo las acciones (f) directamente en los case y la dejo como: void srvKeysMef ( void )
//
// Como demo mando al serie un string con los estados y eventos activados por las teclas/botones/pulsadores
//
/*------------------------------------------------------------------------------------------------------------- */
//void srvKeysMef( void* taskParmPtr ){
void srvKeysMef( void ){
	// Como los estados KEY_ARR_LONG y KEY_ABA_LONG son estacionarios, no permito que se los llame consecutivamente
	// para eso defino bKEY_ABA_LONG[] y bKEY_ARR_LONG[]. Estos arreglos guardan si los estados se usaron en el
	// penúltimo llamado para que si se repiten, inhiban el llamado.
	static bool_t bKEY_ABA_LONG[KEYSMAX], bKEY_ARR_LONG[KEYSMAX];
	void *ptr = NULL;			//El puntero para comunicaciones

	// ---------- REPETIR POR SIEMPRE --------------------------

	//Loop de servicios de la Mef
	for (uint8_t i = 0; i < nKEYS; i++) {

		if ( teclas[i].estado != KEY_ARR_LONG ) bKEY_ARR_LONG[i] = FALSE;
		if ( teclas[i].estado != KEY_ABA_LONG ) bKEY_ABA_LONG[i] = FALSE;

		//Switch de estados. . .
		switch ( teclas[i].estado ){

			case KEY_ARR_SHORT:
				PRINT_DEBUG( ("KEY_ABA_TIME: \t key: %i \t dT: %i \n", i, teclas[i].tiempoDOWN ) );
				//LED_DEBUG( (LEDB + i, OFF ) );
				PRINT_DEBUG( ("KEY_ARR_SHORT: \t key: %i \t gpio: %i\n", i, teclas[i].tecla ) );
			    //TO-DO
				//preparo para enviar
				uint16ToAscii( teclas[i].tiempoDOWN );					//convierto el tiempo en digitos

				//ptr = pvPortMalloc(11);
				ptr = QMPool_get( &miMemPool1, 4 );		// 4 bloques de 5 bytes

				if ( ptr ){
					  *( ( char * ) ptr + 0 ) = 'T';
					  *( ( char * ) ptr + 1 ) = 'E';
					  *( ( char * ) ptr + 2 ) = 'C';
					  *( ( char * ) ptr + 3 ) =  i + 48; 			//pongo el N° de tecla
					  *( ( char * ) ptr + 4 ) = ' ';
					  *( ( char * ) ptr + 5 ) = 'T';
					  *( ( char * ) ptr + 6 ) = NUM[3] + 48;	//pongo los dígitos como ASCII
					  *( ( char * ) ptr + 7 ) = NUM[2] + 48;
					  *( ( char * ) ptr + 8 ) = NUM[1] + 48;
					  *( ( char * ) ptr + 9 ) = NUM[0] + 48;
					  *( ( char * ) ptr + 10) =  0 ;

					  xQueueSend( myQueueCommandHandle, &ptr,  (TickType_t) 1);  //mando el puntero

				}else{
					 // Falla alocacion de memoria;
				}
				break;

			case KEY_ARR_LONG:
				LED_DEBUG( (LEDB + i, OFF ) );
				if ( ! bKEY_ARR_LONG[i] ){			//Inhibiendo en la repetición KEY_ARR_LONG
					   bKEY_ARR_LONG[i]  = TRUE;
					   PRINT_DEBUG( ("KEY_ARR_LONG: \t key: %i \t gpio: %i\n\n", i, teclas[i].tecla ) );
					   //TO-DO
				}
				break;

			case KEY_ABA_SHORT:
				teclas[i].tiempoDOWN = xTiempoMinFall;          //contando el 1er tiempo abajo. Esto tambien es el RESET del contador
				//LED_DEBUG( (LEDB + i, ON) );
				PRINT_DEBUG( ("KEY_ABA_SHORT: \t key: %i \t gpio: %i\n", i, teclas[i].tecla ) );
			     //TO-DO
				break;

			case KEY_ABA_LONG:
				LED_DEBUG( (LEDB + i, ON) );
				if ( bKEY_ABA_LONG[i]  )  {
					teclas[i].tiempoDOWN += xTiempoAntiRebote;  //sumando el 3er y sucesivos tiempos abajo
				}else{
				     bKEY_ABA_LONG[i]  = TRUE;           	   //Inhibo la repetición KEY_ABA_LONG
				     teclas[i].tiempoDOWN += xTiempoMinDown;   //sumando el 2do tiempo abajo
				     PRINT_DEBUG( ("KEY_ABA_LONG: \t key: %i \t gpio: %i\n", i, teclas[i].tecla ) );
				     //TO-DO
				}
				break;

			//Estados transitorios No recomiendo usarlos para TO-DO
			case KEY_FALL:
				vTaskDelay( xTiempoMinFall );
				break;
			case KEY_RISE:
				vTaskDelay( xTiempoMinRise );
				break;
			case KEY_ABA_TRANS:
				vTaskDelay( xTiempoMinDown );
				break;
			case KEY_ARR_TRANS:
				vTaskDelay( xTiempoMinUp );
				break;
		}

		//Switch de eventos. . .
		switch ( teclas[i].evento ){
			case EVENT_KEY_DOWN:
				PRINT_DEBUG( ("EVENT_KEY_DOWN: \t key: %i \t gpio: %i\n", i, teclas[i].tecla ) );
			     //TO-DO
				break;
			case EVENT_KEY_CLICK:
				PRINT_DEBUG( ("EVENT_KEY_CLICK: \t key: %i \t gpio: %i\n", i, teclas[i].tecla ) );
			     //TO-DO
				break;
			case EVENT_KEY_UP:
				PRINT_DEBUG( ("EVENT_KEY_UP: \t key: %i \t gpio: %i\n", i, teclas[i].tecla ) );
			     //TO-DO
				break;
			case EVENT_KEY_2CLICK:
				PRINT_DEBUG( ("\nEVENT_KEY_2CLICK: \t key: %i \t gpio: %i\n\n", i, teclas[i].tecla ) );
			     //TO-DO
				break;
			case EVENT_KEY_NULL:
				//DEBUG_PRINT( ("EVENT_NULL: %i\n", teclas[i].tecla ) );
			     //TO-DO
				break;
		}
	}
}

  /*FINITO*/
