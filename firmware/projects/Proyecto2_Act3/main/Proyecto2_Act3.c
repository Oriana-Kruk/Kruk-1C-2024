/*! @mainpage proyecto 2_act 3
 *
 * @section genDesc General Description
 * 
 * Este programa realiza lo siguiente:
 * 1- Utiliza un sensor para medir distancias y mostrarlas en un display.
 * 2- Controla LEDs según el rango de distancia medida.
 * 3- Envia las mediciones al terminal de una PC a través del puerto serie en un formato específico: 
 *    3 dígitos ASCII de distancia, seguidos de un espacio, dos caracteres para la unidad (cm), y un cambio de línea "\r\n".
 * 4- Permite el control de la medición y la retención de resultados utilizando las teclas "O" y "H", 
 *    replicando la funcionalidad de las teclas 1 y 2 realizadas en la Act 1.
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

/*Modifique la actividad del punto 2 agregando ahora el puerto serie.
 Envíe los datos de las mediciones para poder observarlos en un terminal en la PC, siguiendo el siguiente formato:

3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea “ \r\n”
Además debe ser posible controlar la EDU-ESP de la siguiente manera:
Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP

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

/*==================[macros and definitions]=================================*/
#define UART_NO_INT	0		/*!< Flag used when no reading interruption is required */
uint16_t distancia;

/*==================[internal data definition]===============================*/
// Estas son variables GLOBALES, por lo que
TaskHandle_t led1_task_handle = NULL;

/** @def tecla1
 * @brief comienza y detiene la medicion
 */
bool tecla1 = false;

/** @def tecla2
 * @brief mantiene el valor de la medicion
 */
bool tecla2 = false;

/*==================[internal functions declaration]=========================*/

/** @fn static void TansferirDatos()
 * @brief Envía los datos de la distancia medida a través del puerto serie (UART) hacia un terminal en una PC (UART_PC). 
 * Utiliza la función UartItoa para convertir el valor numérico de la distancia (representado como un entero)
 *  en una cadena de caracteres en formato decimal, que luego es enviada mediante UartSendString. 
 * Además, al final de la cadena de distancia, se envía " cm\r\n" para indicar la unidad de medida (centímetros)
 * y un cambio de línea en el terminal.
 * @param 
 */
//esta funcion es especifica del ej 3
static void TansferirDatos(){
	UartSendString(UART_PC, (char*)UartItoa(distancia, 10)); 
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

/** @fn prende_apaga()
 * @brief Utiliza una variable booleana (tecla1) para alternar su valor cada vez que se llama, 
 * lo que trabaja como interruptor de encendido y apagado.
 * @param 
 */
void prende_apaga()
{
	tecla1 = !tecla1;
}

/** @fn prende_apaga()
 * @brief Utiliza una variable booleana (tecla2) para invertir su estado cada vez que se llama,
 * para mantener o no el resultado medido.
 * @param 
 */
void Hold()
{
	tecla2 = !tecla2;
}

/** @fn void FuncTimerA(void *param)
 * @brief Cuando el temporizador asociado a esta función alcanza su límite de tiempo, se genera una interrupción que activa FuncTimerA.
 * Dentro de esta función, vTaskNotifyGiveFromISR tiene un papel crucial al enviar una notificación importante a la tarea asociada 
 * al LED_1 (led1_task_handle), permitiéndole continuar su ejecución en el momento adecuado.
 * @param .void *param
 */
void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(led1_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

/** @fn static void ObtenerDatos()
 * @brief La función ObtenerDatos lee un byte de datos del puerto serie (UART_PC). Luego, utiliza una estructura de selección
 * (switch) para verificar qué caracter se ha recibido. Si el caracter es 'O', se llama a la función prende_apaga, que realiza
 * la acción de encendido o apagado. Si el caracter es 'H', se llama a la función Hold, que mantiene el valor medido de la distancia.
 * @param 
 */
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