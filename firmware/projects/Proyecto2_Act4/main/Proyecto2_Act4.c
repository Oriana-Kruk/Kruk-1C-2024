/*! @mainpage Proyecto2_Act4
 *
 * @section genDesc General Description
 *
 * El codigo utiliza un conversor analógico-digital (ADC) para digitalizar una señal analógica en CH1 y la envía por UART
 * a un graficador en la PC. Además, convierte una señal digital del ECG en una señal analógica y la envía por el mismo medio,
 * utilizando temporizadores para notificar las tareas de lectura y envío de datos por UART.
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
 * @author Kruk Oriana
 *
 */
/* Actividad 4
Diseñar e implementar una aplicación, basada en el driver analog_io_mcu.h y el driver de transmisión serie uart_mcu.h,
que digitalice una señal analógica y la transmita a un graficador de puerto serie de la PC. Se debe tomar la entrada CH1
del conversor AD y la transmisión se debe realizar por la UART conectada al puerto serie de la PC, en un formato compatible
con un graficador por puerto serie.
*/
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"

#define TIME_PERIOD 2000
#define TIME_PERIOD2 4000

uint8_t indice = 0;
/*==================[internal data definition]===============================*/
#define BUFFER_SIZE 231

/*==================[internal data definition]===============================*/
TaskHandle_t main_task_handle = NULL;
// Lista de valores para la señal ECG
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/
TaskHandle_t task_handle1 = NULL;
TaskFunction_t task_handle2 = NULL;

 /** @fn  void Notify(void *param)
  * @brief  notifica a las tareas task_handle1 y task_handle2
  * @param *param
 */
void Notify(void *param)
{
	vTaskNotifyGiveFromISR(task_handle1, pdFALSE);
	vTaskNotifyGiveFromISR(task_handle2, pdFALSE);
}


 /** @fn  static void deAnalogico_aDigital(void *pvParameter)
  * @brief  espera una notificación, lee el valor analógico del canal 1, lo convierte a una cadena de caracteres y lo envía por UART.
  * Convirtiendo de una senal analogica a una digital.
  * @param *pvParameter
 */
static void deAnalogico_aDigital(void *pvParameter)
{
	uint16_t valorDigitalizado; //almacenar el valor digitalizado de la señal analógica
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // La tarea espera en este punto hasta recibir una notificación 
		//se lee el valor analógico del canal 1 (CH1) utilizando la función AnalogInputReadSingle y se almacena en la variable valorDigitalizado
		AnalogInputReadSingle(CH1, &valorDigitalizado);
		//convierte el valor digitalizado valorDigitalizado en una cadena de caracteres utilizando 'UartItoa' con una base decimal (10) 
		UartSendString(UART_PC, (char *)UartItoa(valorDigitalizado, 10));
		//envía esa cadena por UART utilizando 'UartSendString' a la UART conectada al puerto serie de la PC (UART_PC).
		UartSendString(UART_PC, (char *)"\r\n");//Después de enviar el valor digitalizado, se envía una secuencia de retorno de carro (\r) y salto de línea (\n) por UART, lo que indica un final de línea 
	}
}


 /** @fn  static void deDigital_aAnalogico(void *pvParamete)
  * @brief Convierte una señal digital de un ECG (provista por la cátedra) en una señal analógica
  * @param *pvParameter
 */
static void deDigital_aAnalogico(void *pvParamete)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);// la tarea espera hasta que se reciba una notificación.

		AnalogOutputWrite(ecg[indice]);//convierte de digital a analogico
		indice++; //En cada iteración se incrementa el indice para avanzar al siguiente elemento de ecg  
		//Si el valor de indice es igual al tamaño de ecg, significa que se ha alcanzado el final de la matriz. 
		//En este caso, se reinicia el índice a 0 para empezar a leer ecg desde el principio nuevamente.
		if (indice == sizeof(ecg))
		{
			indice = 0;
		}
		
}}
/*==================[external functions definition]==========================*/
void app_main(void)
{

	analog_input_config_t analogInput1 = {
		.input = CH1,		// Se configura para leer del canal 1 del conversor analógico-digital (ADC)
		.mode = ADC_SINGLE, // Se configura para realizar una única lectura analógica
	};
	analog_input_config_t analogInput2 = {
		.input = CH0,        // Se configura para leer del canal 0 del conversor analógico-digital (ADC)
		.mode = ADC_SINGLE,  // Se configura para realizar una única lectura analógica
	};
	
	// inicialización de timers
	timer_config_t timer_1 = {
		.timer = TIMER_A,
		.period = TIME_PERIOD,
		.func_p = Notify ,
		.param_p = NULL};
	timer_config_t timer_2 = {
		.timer = TIMER_B,
		.period = TIME_PERIOD,
		.func_p = Notify,
		.param_p = NULL};
	//configura los parámetros de comunicación serial, específicamente para la UART 
	serial_config_t serial_global = {
		.port = UART_PC, //indica el puerto
		.baud_rate = 115200,//velocidad de transmisión de datos en baudios
		.func_p = NULL,
		.param_p = NULL};

	TimerInit(&timer_1); //Inicializa el temporizador 1 con la configuración previamente definida en timer_1
	TimerInit(&timer_2); //Inicializa el temporizador 2 con la configuración previamente definida en timer_2
	AnalogOutputInit(); //Inicializa la salida analógica
	AnalogInputInit(&analogInput1); // Inicializa la entrada analógica utilizando la configuración proporcionada en analogInput1.
	xTaskCreate(&deAnalogico_aDigital, "leer y enviar", 2048, NULL, 5, &task_handle1);//Crea una tarea llamada deAnalogico_aDigital
	                                                                                  //El puntero task_handle1 es una bandera se utiliza para almacenar el identificador de la tarea creada.
	xTaskCreate(&deDigital_aAnalogico, "leer y enviar", 2048, NULL, 5, &task_handle2);
	UartInit(&serial_global);//Inicializa la comunicación serial UART utilizando la configuración proporcionada en serial_global
	TimerStart(timer_1.timer);// Inicia el temporizador para que comience a contar según su configuración.
	TimerStart(timer_2.timer);
}
/*==================[end of file]============================================*/