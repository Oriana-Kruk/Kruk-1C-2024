/*! @mainpage proyecto 2_act 3
 *
 * @section genDesc General Description
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
#include "timer_mcu.h"
#include "uart_mcu.h"

// #define HC_SR04_H
// #define HC_SR04_H

/*==================[macros and definitions]=================================*/
//#define CONFIG_BLINK_PERIOD 1000 // para las mediciones
//#define CONFIG_BLINK_PERIOD_2 50 // para las teclas
#define UART_NO_INT	0		/*!< Flag used when no reading interruption is required */
uint16_t distancia;

/*==================[internal data definition]===============================*/
// Estas son variables GLOBALES, por lo que
TaskHandle_t led1_task_handle = NULL;
//TaskHandle_t led1_task_handle = NULL;

/** @def tecla1
 * @brief comienza y detiene la medicion
 */
bool tecla1 = false;

/** @def tecla2
 * @brief mantiene el valor de la medicion
 */
bool tecla2 = false;

/*==================[internal functions declaration]=========================*/

//esta funcion es especifica del ej 3
static void TansferirDatos(){
	UartSendString(UART_PC, (char*)UartItoa(distancia, 10)); //el UartSendString envia una cadena a raves del puerto serie, los datos se envian a UART_PC
	//y la cadena que se envia es la que el UartItoa convierte la distancia medida en un string, "distancia" es el numero a ser convertido y la base de coonvercion es 10(decimal)
	UartSendString(UART_PC, " cm\r\n");
}
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
	while (1)
	{ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		if (tecla1)
		{
			distancia = HcSr04ReadDistanceInCentimeters();

			TansferirDatos();
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
		//vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

void prende_apaga()
{
	tecla1 = !tecla1;
}

void Hold()
{
	tecla2 = !tecla2;
}
void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(led1_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

static void ObtenerDatos(){
	uint8_t data;
	UartReadByte(UART_PC, &data); //UART_PC puerto usb  y data va a los datos almacenados
	switch (data)
	{
		case 'O':
			prende_apaga();
			break;
		case 'H':
			Hold();
			break;
	}
}


/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	LcdItsE0803Init();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit();

	SwitchActivInt(SWITCH_1, &prende_apaga, NULL);
	SwitchActivInt(SWITCH_2, &Hold, NULL);

	timer_config_t my_timer={
		.timer = TIMER_A,
		.period = 1000000,
		.func_p = FuncTimerA,
		.param_p = NULL
	};
	TimerInit(&my_timer);

	serial_config_t my_uart = {
		.port = UART_PC,  //puerto
		.baud_rate = 9600, //bits por segundo
		.func_p = &ObtenerDatos, //puntero a funcion que obtiene los datos
		.param_p = NULL 
	};
	UartInit(&my_uart);
	

	xTaskCreate(&Task_MostrarDistancia_enDisplay, "distancias_task", 512, NULL, 5, &led1_task_handle);

		TimerStart(my_timer.timer);

}
/*==================[end of file]============================================*/