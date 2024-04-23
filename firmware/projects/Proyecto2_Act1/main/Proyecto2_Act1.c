/*! @mainpage proyecto 2_act 1
 *
 * @section genDesc General Description
 *
 *  Se mide la distancia con el sensor y lo muestra por mantalla y marca rangos de distancias medida utilizando los leds.
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * 
 * Al presionar la tecla 1 se prende y apaga la EDU-ESP y al presionar la tecla 2 se mantiene la medida en el display 
 * pero sigue marcando las distancias con los leds
 * 
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |  EDU-CIAA-NXP  |    Peripheral 	|
 * |:--------------:|:------------------|
 * | 	GPIO_2	 	|  	  ECHO  	   	|
 * | 	GPIO_3	 	|  	  TRIGGER 	   	|
 * | 	+5V 	 	|  	  +5V   	   	|
 * | 	GND 	 	|  	  GND   	   	|
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
#include "switch.h"
#include "gpio_mcu.h"
#include "lcditse0803.h" //para usar el lcd
#include "hc_sr04.h"	 // para usar el que mide la distancia
#include "delay_mcu.h"

// #define HC_SR04_H
// #define HC_SR04_H

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000 // para las mediciones
#define CONFIG_BLINK_PERIOD_2 50 // para las teclas

/*==================[internal data definition]===============================*/
// Estas son variables GLOBALES, por lo que

/** @def tecla1
 * @brief comienza y detiene la medicion
*/
bool tecla1 = false;

/** @def tecla2
 * @brief mantiene el valor de la medicion
*/
bool tecla2 = false; 

/*==================[internal functions declaration]=========================*/

/** @fn void Task_MostrarDistancia_enDisplay(uint16_t distancia)
 * @brief Mide la distancia con el sensor y lo muestra por mantalla y marca rangos de distancias medida utilizando los leds.
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * @param .void *pvParameter
 */
void Task_MostrarDistancia_enDisplay(void *pvParameter) // ver lo de *pvParameter
{
	uint16_t distancia;
	while (1)
	{
		if (tecla1)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
			if (!tecla2)
			{
				LcdItsE0803Write(distancia);
			}

			if (distancia < 10)
			{
				LedsOffAll();
			}
			else if (distancia < 20)
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if (distancia < 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
		}
		else
		{
			LedsOffAll();
			LcdItsE0803Off();

		}
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

/** @fn void Task_switches(void *pvParameter)
 * @brief Al presionar la tecla 1 prende y apaga la EDU-CIAA y al presionar la tecla 2  mantiene la medicion en el display
 * pero sigue marcando los rangos de distancias con los leds
 * @param void *pvParameter
 */
void Task_switches(void *pvParameter)
{
	uint16_t teclas;

	while (1)
	{
		teclas = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			tecla1 = !tecla1; // tecla 1 para activar y detener la medicion
			// !true significa que si esta en true lo pone en false y si esta en false lo pone en true, porque primero lo inicialice en false
			break;
		case SWITCH_2:
			tecla2 = !tecla2; // tecla 2 para mantener el resultado HOLD
			break;
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_2 / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();

	xTaskCreate(&Task_MostrarDistancia_enDisplay, "distancias_task", 512, NULL, 5, NULL);
	xTaskCreate(&Task_switches, "switchTeclas_task", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/