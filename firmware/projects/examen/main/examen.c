/*! @mainpage Examen
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
 * | 	Bomba Agua	 	|   GPIO_23     |
 * | 	Bomba pha	 	| 	GPIO_22 	|
 * | 	Bomba phb	 	| 	GPIO_21 	|
 * | 	sensor humedad	| 	GPIO_20     |
 * | 	sensor ph	 	| 	CH1 		|
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

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "gpio_mcu.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
#define GPIO_BOMBA_AGUA GPIO_23
#define GPIO_BOMBA_PHA GPIO_22
#define GPIO_BOMBA_PHB GPIO_21
#define SENSOR_HUMEDAD GPIO_20
#define SENSOR_PH CH1

#define VOLTAJE_MIN 0.0
#define VOLTAJE_MAX 3300
#define PH_MIN 6.0
#define PH_MAX 6.7
#define humedad

/*==================[internal data definition]===============================*/
TaskHandle_t sumnistroaAgua_task_handle = NULL;
TaskHandle_t medirPH_task_handle = NULL;
bool start;
/*==================[internal functions declaration]=========================*/
/** @fn void SuministrarAgua()
 * @brief Activa la bomba para para suministrar agua.
 */
void SuministrarAgua()
{
	GPIOOn(GPIO_BOMBA_AGUA); // Prender Bomba agua
}
/** @fn void sensorPHA()
 * @brief Activa la bomba de pha
 */
void sensorPHA()
{
	GPIOOn(GPIO_BOMBA_PHA); // Prender Bomba pha
}
/** @fn void sensorPHB()
 * @brief Activa la bomba de pha
 */
void sensorPHB()
{
	GPIOOn(GPIO_BOMBA_PHB); // Prender Bomba phb
}
/** @fn void sensor_humedad()
 * @brief pone en alto el sensor de humedad
 */
void sensor_humedad()
{
	if (humedad=0)
	{
		GPIOOff(SENSOR_HUMEDAD);
	}
	else if (humedad=1)
	{
		GPIOOn(SENSOR_HUMEDAD);
	}
	
}

/** @fn void Stop()
 * @brief al utilizar la tecla 2 se detiene el sitema
 */
void Stop()
{
	start = false;
}
/** @fn void Start()
 * @brief al utilizar la tecla 1 se inicia el sitema
 */
void Start()
{
	start = true;
}

/** @fn uint16_t ConvertirVoltajeA_PH(uint16_t voltaje)
 * @brief Convierte un valor de voltaje a ph
 */
uint16_t ConvertirVoltajeA_PH(uint16_t voltaje)
{
	return PH_MIN + (voltaje - VOLTAJE_MIN) * (PH_MAX - VOLTAJE_MIN) / (VOLTAJE_MAX - VOLTAJE_MIN);
}

/** @fn void MensajeUART(uint8_t mensaje)
 * @brief maneja los mensajes de la UART
 */
void MensajeUART(uint8_t mensaje) // mensajes a traves de la uart
{ 
	switch (mensaje)
	{
	case 1:
		UartSendString(UART_PC, "humedad correcta\r\n");
		break;
	case 2:
		UartSendString(UART_PC, " “humedad incorrecta”\r\n");
		break;
	case 3:
		UartSendString(UART_PC, " “Bomba de pHA encendida”\r\n");
		break;
	case 4:
		UartSendString(UART_PC, " “Bomba de pHB encendida”\r\n");
		break;
	case 5:
		UartSendString(UART_PC, " “Bomba de agua encendida”\r\n");
		break;
	default:
		break;
	}
}

/** @fn void Task_SuministoAgua(void *pvParameter)
 * @brief maneja el suministro de humedad, activando una bomba de agua en caso que se necesite
 */
void Task_SuministoAgua(void *pvParameter)
{
	uint16_t humedad;
	
	if (humedad=0)
	{
		MensajeUART(1);
	}
	else if (humedad=1)
	{
		SuministrarAgua();
		MensajeUART(2);
		MensajeUART(5);

	}	
}

/** @fn void Task_MedirPH(void *pvParameter)
 * @brief maneja la activacion de las bombas de ph en caso de ser necesario, ya que sensa los valores de ph
 */
void Task_MedirPH(void *pvParameter)
{
    uint16_t sensor_ph=0; 
    uint16_t PHamV=0;
	uint16_t ph=0;
while (1)
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	if (start)
	AnalogInputReadSingle(GPIO_BOMBA_PHA, &PHamV); // Lee el valor de la PH en voltaje
			ph =  ConvertirVoltajeA_PH(PHamV);
	{
		if (sensor_ph < 6 )
		{
			sensorPHB();
			MensajeUART(4);
		}
		else if (sensor_ph >=6 && sensor_ph <6.7 )
		{
			GPIOOff(GPIO_BOMBA_PHB);
		}
		else if (sensor_ph >=6.7)
		{
		    sensorPHA();
			MensajeUART(3);
		}
	}
}
}


/*==================[external functions definition]==========================*/
void app_main(void){
int8_t SwitchesInit(void);

SwitchActivInt(SWITCH_1, &Start, NULL);
SwitchActivInt(SWITCH_2, &Stop, NULL);
//para la bomba de agua
	GPIOInit(GPIO_BOMBA_AGUA, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA_AGUA);

//para la bomba de pha
	GPIOInit(GPIO_BOMBA_PHA, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA_PHA);

//para la bomba de phb
	GPIOInit(GPIO_BOMBA_PHB, GPIO_OUTPUT);
	GPIOOff(GPIO_BOMBA_PHB);

	// Para la uart
	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 115200, /*!< baudrate (bits per second) */
		.func_p = NULL,		 /*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
		.param_p = NULL		 /*!< Pointer to callback function parameters */
	};
	UartInit(&my_uart);

	xTaskCreate(&Task_SuministoAgua, "Task_suministroAgua", 2048, NULL, 5, &sumnistroaAgua_task_handle);
	xTaskCreate(&Task_MedirPH, "Task_MedirPH", 2048, NULL, 5, &medirPH_task_handle);
}
/*==================[end of file]============================================*/