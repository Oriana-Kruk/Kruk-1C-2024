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
 * |    Peripheral     |     ESP32   	|
 * |:-----------------:|:---------------|
 * | 	  Si7007	   |   CH1 y CH2	|
 * | detector de linea | 	GPIO_23		|
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
#include "led.h"
#include "ble_mcu.h"
#include "math.h"

/*==================[macros and definitions]=================================*/
float temperature = 0;
float humidity = 0;
float vel_viento = 0;
#define CONFIG_BLINK_PERIOD 500
#define LED_BT LED_1

/*==================[internal data definition]===============================*/
uint16_t contador = 0;
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
/** @fn static void medir_tempertatura_y_humedad(void *pvParameter);
 * @brief mide temperatura y humedad a traves del sensor si7007 y convierte de analogico a digital (lo que ocurre en el driver)
 * Ademas envia los datos de temperatura y humedad por bluetooth
 * @param void *pvParameter
 */
static void medir_tempertatura_y_humedad(void *pvParameter)
{
    float temperature, humidity;
    char msg[100]; // para lo que se manda por bluetooth
    while (true)
    {
        // Medir la temperatura y humedad utilizando el sensor Si7007
        temperature = Si7007MeasureTemperature();
        humidity = Si7007MeasureHumidity();

        // para ver si mide bien por el monitor device
        printf("valores  de temperatura %f, humedad %f \n", temperature, humidity);
        sprintf(msg, "*T%d", (int)temperature); // barra de temperatura
        BleSendString(msg);
        sprintf(msg, "*B%d", (int)humidity); // barra de humedad
        BleSendString(msg);

        // Esperar un tiempo antes de realizar la siguiente medición
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Esperar 10 minutos (600000 ms) // deberia ponerlo menos para poder probar
    }
}
/** @fn static void medir_velocidad(void *pvParameter)
 * @brief mide las revoluciones por segundo del anemometro con un detector de linea y luego realiza el calculo
 * de la velocidad del viento y envia dicho dato por bluetooth
 * @param void *pvParameter
 */
static void medir_velocidad(void *pvParameter)
{
    float revol_por_seg;
    float vel_viento;
    char msg[100]; // para lo que se manda por bluetooth
    while (true)
    {
        revol_por_seg = (float)contador / 8.0;

        // ec para pasar de revol por segundo a velocidad lineal (m/seg)   -> vel=2*pi*radio (metros)*revol por seg
        vel_viento = 2 * 3.14 * 0.0225 * revol_por_seg;

        printf("velocidad del viento %f \n", vel_viento);

        sprintf(msg, "*D%d", (int)(100 * vel_viento)); // barra de velocidad del viento
        BleSendString(msg);
        contador = 0;

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void ContarLineas()
{
    contador++;
}
/** @fn static void deteccion(void *pvParameter)
 * @brief se encarga de enviar una alarma "RIESGO DE INCENDIO" en caso de que los valores de temperatura,
 * humedad y viento se encuentren en un rango en el que haya riesgo de incendio 
 * @param void *pvParameter
 */
static void deteccion(void *pvParameter)
{
    char msg[100]; // para lo que se manda por bluetooth

    while (true)
    {
        // en la vida real seria temp>30`C hum<30%  viento>20km/h pero utilizo valores que pueda probar en el aula
        if (temperature > 20 && humidity < 60 && vel_viento > 0.1)
        {
            printf("RIESGO DE INCENDIO \n");
            BleSendString(msg);

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
void read_data(uint8_t *data, uint8_t length) {}

void app_main(void)
{
    GPIOInit(GPIO_23, GPIO_INPUT); // inicializo el contador de lineas
    GPIOActivInt(GPIO_23, ContarLineas, true, NULL);

    LedsInit();

    // configura el sensor Si7007
    Si7007_config my_sensor = {
        .select = GPIO_9,
        .PWM_1 = CH1,
        .PWM_2 = CH2,
    };
    Si7007Init(&my_sensor); // Inicializar el sensor Si7007

    ble_config_t ble_configuration = {
        "Alarma incendios",
        read_data};
    BleInit(&ble_configuration);

    // Creo las tareas
    xTaskCreate(&medir_tempertatura_y_humedad, "medir tempertatura y humedad", 2048, NULL, 5, NULL);
    xTaskCreate(&medir_velocidad, "medir velocidad del viento", 2048, NULL, 5, NULL);
    xTaskCreate(&deteccion, "alerta deteccion peligrosa", 2048, NULL, 5, NULL);

    // para ver si se conecto el bluetooth
    while (1)
    {
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        switch (BleStatus())
        {
        case BLE_OFF:
            LedOff(LED_BT);
            break;
        case BLE_DISCONNECTED:
            LedToggle(LED_BT);
            break;
        case BLE_CONNECTED:
            LedOn(LED_BT);
            break;
        }
    }
}

/*==================[end of file]============================================*/