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
#include "uart_mcu.h"


#define TIME_PERIOD 1000000 //1 segundo
#define TIME_PERIOD2 100000 // 100 ms

#define SENSOR_TEMPERATURA CH1

#define VOLTAJE_MIN 0.0
#define VOLTAJE_MAX 3300
#define TEMPERATURA_MIN 20.0
#define TEMPERATURA_MAX 50.0
/*==================[macros and definitions]=================================*/
uint16_t distancia;
float promedio;
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
uint16_t ConvertirVoltajeATemperatura(uint16_t voltaje)
{
	return TEMPERATURA_MIN + (voltaje - VOLTAJE_MIN) * (TEMPERATURA_MAX - VOLTAJE_MIN) / (VOLTAJE_MAX - VOLTAJE_MIN);
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

void medirTemperatura(void *pvParameter)
{
	uint16_t temperatura_mV;
	uint16_t temperatura;

	int numMediciones = 10;
    float mediciones[numMediciones];
	float suma = 0.0;

	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (distancia >=8 && distancia <=12)
	{
		 for(int i = 0; i < numMediciones; i++)
		{
		AnalogInputReadSingle(SENSOR_TEMPERATURA, &temperatura_mV); 
		temperatura = ConvertirVoltajeATemperatura(temperatura_mV);

        mediciones[i] = temperatura;
		suma += mediciones[i];
		float promedio = suma / numMediciones;
	    }	

	    // MESAJES POR LA UART
		UartSendString(UART_PC, (char *) UartItoa(promedio, 10));
        UartSendString(UART_PC, "Cº");
        UartSendString(UART_PC, (char *) UartItoa(distancia, 10));
        UartSendString(UART_PC, "cm\r\n");
	 
	}
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){

	// configuro timer 1 para el control de las distancias
	timer_config_t timer_1 = {
		.timer = TIMER_A,
		.period = TIME_PERIOD,
		.func_p = Notify,
		.param_p = NULL};

	TimerInit(&timer_1); // inicializo timer1

		timer_config_t timer_2 = {
		// configuro timer 2 para el control de la temperatura
		.timer = TIMER_B,
		.period = TIME_PERIOD2,
		.func_p = Notify2,
		.param_p = NULL};

	TimerInit(&timer_2); // inicializo timer 2

	TimerStart(timer_1.timer); // para que comience el timer 1
	TimerStart(timer_2.timer); // para que comience el timer 2

	HcSr04Init(GPIO_20, GPIO_21); 
	GPIOInit(SENSOR_TEMPERATURA, GPIO_OUTPUT);

	LedsInit();

xTaskCreate(&medirDistancia, "medir distancias", 2048, NULL, 5, &task_handle); 
xTaskCreate(&medirTemperatura, "medir temperatura", 2048, NULL, 5, &task_handle2); 
}
/*==================[end of file]============================================*/