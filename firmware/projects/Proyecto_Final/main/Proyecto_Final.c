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
#include "led.h"
#include "ble_mcu.h"

/*==================[macros and definitions]=================================*/
float temperature =0;
float humidity =0;
float vel_viento =0;
#define CONFIG_BLINK_PERIOD 500
#define LED_BT	LED_1

/*==================[internal data definition]===============================*/
uint16_t contador = 0;
/*==================[internal functions declaration]=========================*/

/** @fn static void medir_tempertatura_y_humedad(void *pvParameter);
 * @brief mide temperatura y humedad y convierte de analogico a digital
 * @param void *pvParameter
 */


/*==================[external functions definition]==========================*/

static void medir_tempertatura_y_humedad(void *pvParameter)
{
    float temperature, humidity;
char msg[10];
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

//ec para pasar de revol por segundo a velocidad lineal (m/seg)   -> vel=2*pi*radio (metros)*revol por seg
        vel_viento= 2 * 3.14 * 0.0225 * revol_por_seg;

        printf("revoluciones por segundo %f \n", revol_por_seg);
        printf("cont de lineas %d \n", contador);
        printf("velocidad del viento %f \n", vel_viento);

        contador = 0;

        vTaskDelay(1000/portTICK_PERIOD_MS); 
    }
}
void ContarLineas(){
    contador++;
}

static void deteccion(void *pvParameter)
{
    char msg[10]; // para lo que se manda por bluetooth

    while (true)
    {
    medir_tempertatura_y_humedad(pvParameter);
    medir_velocidad(pvParameter);

 //ver con que valores pruebo en la vida real seria temp>30`C hum<30%  viento>20km/h
   // if (temperature>20 && humidity<100 && vel_viento>0.6)
    
      //lo que se manda por bluetooth
      // BleSendString("*f RIESGO DE INCENDIO"); //mensaje de riesgo

        printf ( "RIESGO DE INCENDIO");
       sprintf(msg, "*f RIESGO DE INCENDIO*"); 
       BleSendString(msg);
     
       sprintf(msg, "*T%.2f*", temperature);  // barra de temperatura
       BleSendString(msg);
       sprintf(msg, "*B%.2f*", humidity);    // barra de humedad 
       BleSendString(msg);
       sprintf(msg, "*D%.2f*", vel_viento);    // barra de velocidad del viento 
       BleSendString(msg);
    
        vTaskDelay(1000/portTICK_PERIOD_MS); 
    }
}
void read_data(uint8_t * data, uint8_t length){}

void app_main(void)
{
    GPIOInit(GPIO_23, GPIO_INPUT); //inicializo el contador de lineas
    GPIOActivInt(GPIO_23, ContarLineas, true, NULL);

    LedsInit();

    // configura el sensor Si7007
       Si7007_config my_sensor = {
        .select = GPIO_9,
       .PWM_1 = CH1,
        .PWM_2 = CH2,
    };
    Si7007Init(&my_sensor);  // Inicializar el sensor Si7007

    ble_config_t ble_configuration = {
        "Alarma incendios",
        read_data
    }; 
    BleInit(&ble_configuration);

    // Creo las tareas
   xTaskCreate(&medir_tempertatura_y_humedad, "medir tempertatura y humedad", 2048, NULL, 5, NULL);
    xTaskCreate(&medir_velocidad, "medir velocidad del viento", 2048, NULL, 5, NULL);
    xTaskCreate(&deteccion, "alerta deteccion peligrosa", 2048, NULL, 5, NULL);

while(1){
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        switch(BleStatus()){
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