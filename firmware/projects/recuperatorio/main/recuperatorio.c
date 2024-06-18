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
 * |    Peripheral      |   ESP32   	            |
 * |:--------------:    |:--------------------------|
 * | 	HcSr04 	        | 	GPIO_20 y GPIO_21		|
 * | SENSOR_TEMPERATURA |       	CH1		        |
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
#include "timer_mcu.h"
#include "hc_sr04.h"
#include "led.h" 
#include "analog_io_mcu.h"


#define TIME_PERIOD 1000000 //1 segundo

#define SENSOR_TEMPERATURA CH1
/*==================[macros and definitions]=================================*/
uint16_t distancia;
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
TaskHandle_t task_handle = NULL;
TaskHandle_t task_handle2 = NULL;

void Notify(void *param)
{
	vTaskNotifyGiveFromISR(task_handle, pdFALSE);
}
void Notify2(void *param)
{
	vTaskNotifyGiveFromISR(task_handle2, pdFALSE);
}


void medirDistancia(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // La tarea espera en este punto hasta recibir una notificación

		distancia = HcSr04ReadDistanceInCentimeters(); 

		if (distancia < 8)
		{
			LedOn(LED_1);
		}
		else if (distancia >=8 && distancia <=12)
		{
			LedOn(LED_2);
		}	
		else 
		LedOn(LED_3);
		
	}
}		

void medirTemeperatura(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // La tarea espera en este punto hasta recibir una notificación
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){

	// configuro timer 
	timer_config_t timer_1 = {
		.timer = TIMER_A,
		.period = TIME_PERIOD,
		.func_p = Notify,
		.param_p = NULL};

	TimerInit(&timer_1); // inicializo timer
	TimerStart(timer_1.timer); // para que comience el timer 1

	HcSr04Init(GPIO_20, GPIO_21); 
	GPIOInit(SENSOR_TEMPERATURA, GPIO_OUTPUT);

	LedsInit();

xTaskCreate(&medirDistancia, "medir distancias", 2048, NULL, 5, &task_handle); 
xTaskCreate(&medirTemperatura, "medir temperatura", 2048, NULL, 5, &task_handle2); 
}
/*==================[end of file]============================================*/