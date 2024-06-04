/*! @mainpage Proyecto_Final
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
 * @author Kruk Oriana
 *
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "si7007.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"

#include <stdbool.h>
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/
//Si7007_config my_sensor;
/*==================[internal data definition]===============================*/
uint16_t contador = 0;
/*==================[internal functions declaration]=========================*/

/** @fn static void medir_tempertatura_y_humedad(void *pvParameter);
 * @brief mide temperatura y humedad y convierte de analogico a digital
 * @param void *pvParameter
 */
static void medir_tempertatura_y_humedad(void *pvParameter);

/*==================[external functions definition]==========================*/

static void medir_tempertatura_y_humedad(void *pvParameter)
{
    float temperature, humidity;

    while (true)
    {
        // Medir la temperatura y humedad utilizando el sensor Si7007
        temperature = Si7007MeasureTemperature();
        humidity = Si7007MeasureHumidity();

        //para ver si mide bien por el monitor device
        printf("valores  de temperatura %f, humedad %f \n", temperature, humidity);

        // Esperar un tiempo antes de realizar la siguiente medición
        vTaskDelay(1000/portTICK_PERIOD_MS); // Esperar 10 minutos (600000 ms) // deberia ponerlo menos para poder probar 
    }
}


static void medir_velocidad(void *pvParameter)
{
    float revol_por_seg;
    float vel_viento;

    while (true)
    {
        revol_por_seg= (float)contador/8.0;

        //aca va la cuenta para calcular vel lineal a partir de las revol por seg

        printf("revoluciones por segundo %f \n", revol_por_seg);
        printf("cont de lineas %d \n", contador);

        contador = 0;

        vTaskDelay(1000/portTICK_PERIOD_MS); 
    }
}
void ContarLineas(){
    contador++;
}

void app_main(void)
{
  // Inicializacion de sensores
    
    //Tcrt5000Init(gpio_t dout)
    GPIOInit(GPIO_23, GPIO_INPUT);
    GPIOActivInt(GPIO_23, ContarLineas, true, NULL);
    // configura el sensor Si7007
    Si7007_config my_sensor = {
        .select = GPIO_9,
        .PWM_1 = CH1,
        .PWM_2 = CH2,
    };
    Si7007Init(&my_sensor);  // Inicializar el sensor Si7007

    // Crear la tarea que será notificada por la interrupción del sensor de temp y humedad
    xTaskCreate(&medir_tempertatura_y_humedad, "medir tempertatura y humedad", 2048, NULL, 5, NULL);
    xTaskCreate(&medir_velocidad, "medir velocidad del viento", 2048, NULL, 5, NULL);


}

/*==================[end of file]============================================*/