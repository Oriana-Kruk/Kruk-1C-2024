/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
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
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
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
bool tecla1 = false; // comienza y detiene la medicion
bool tecla2 = false; // mantiene el valor de la medicion

/*==================[internal functions declaration]=========================*/

/** @fn void Task_MostrarDistancia_enDisplay(uint16_t distancia)
 * @brief Mide y muestra la distancia medida utilizando los leds.
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * Luego muestra el valor de distancia en cm utilizando el display LCD.
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
 * @brief Al presionar la TEC1 activar la medicion y al volver a presionar la TEC1 detener la medición.
 * Al presionar la TEC2 mantiene el resultado (“HOLD”).
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