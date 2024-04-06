/*! @mainpage guia1_ej3
 *
 * @section genDesc General Description
 *
 * Este programa define una estructura leds para controlar el estado y comportamiento de LEDs,
 * con modos de encendido (ON), apagado (OFF), y alternancia (TOGGLE). La función controlLeds recibe un puntero 
 * a esta estructura y, según el modo especificado, enciende, apaga o alterna el estado de un LED específico 
 * durante un número de ciclos y un periodo de tiempo determinados. En la función app_main, se inicializa un LED 
 * en modo ON con 10 ciclos y un periodo de 5 unidades, controlado por la función controlLeds.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Oriana Kruk
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define ON 1 
#define OFF 0
#define TOGGLE 3
#define CONFIG_BLINK_PERIOD 500

/*==================[internal data definition]===============================*/

 /** @struct struct leds my_leds
  * @brief La estructura leds define un conjunto de variables para controlar LEDs. mode especifica el modo de operación 
  * (ON, OFF, TOGGLE), n_led indica el número del LED a controlar, n_ciclos determina la cantidad de ciclos de encendido/apagado,
  * y periodo establece la duración de cada ciclo. Por ejemplo, my_leds puede usarse para controlar un LED específico con cierto 
  * comportamiento en ciclos y duración definidos.
 */
struct leds
{
    uint8_t mode;      /*  ON, OFF, TOGGLE*/
	uint8_t n_led;      /*  indica el número de led a controlar*/
	uint8_t n_ciclos;   /* indica la cantidad de ciclos de ncendido/apagado*/
	uint16_t periodo;   /* indica el tiempo de cada ciclo*/
} my_leds; 

/*==================[internal functions declaration]=========================*/
 /** @fn void controlLeds(struct leds *leds_ptr)
  * @brief recibe un puntero a una estructura leds que contiene información sobre el modo de operación, 
  * el número de LED a controlar, la cantidad de ciclos y el periodo de tiempo. Según el modo especificado 
  * en la estructura, la función enciende (ON), apaga (OFF) o alterna (TOGGLE) el estado del LED correspondiente 
  * durante un número de ciclos y un periodo de tiempo determinados.
  * @param .struct leds *leds_ptr
 */
void controlLeds(struct leds *leds_ptr)
{
	switch(leds_ptr -> mode )
	{
		case ON:
			if(leds_ptr ->n_led == LED_1){
				LedOn(LED_1);
			}
			else if(leds_ptr->n_led ==LED_2){
				LedOn(LED_2);
			}
			else if(leds_ptr->n_led ==LED_3){
				LedOn(LED_3);
			}
		break;	
			case OFF:
			if(leds_ptr ->n_led == LED_1){
				LedOff(LED_1);
			}
			else if(leds_ptr->n_led ==LED_2){
				LedOff(LED_2);
			}
			else if(leds_ptr->n_led ==LED_3){
				LedOff(LED_3);
			}
		break;
			case TOGGLE:
			for(int i=0; i<leds_ptr->n_ciclos; i++){
			if(leds_ptr->n_led == LED_1)
				LedToggle(LED_1);
			else if(leds_ptr->n_led == LED_2)
				LedToggle(LED_2);
			else if(leds_ptr->n_led == LED_3)
				LedToggle(LED_3);
			for(int j=0; j<leds_ptr->periodo; j++){
				vTaskDelay(CONFIG_BLINK_PERIOD/ portTICK_PERIOD_MS);
			}
			}
		break;
	}
}


/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	struct leds my_leds;
	my_leds.mode=1;
	my_leds.n_led=LED_2;
	my_leds.n_ciclos=10;
	my_leds.periodo=5;
	controlLeds(&my_leds);

}
/*==================[end of file]============================================*/