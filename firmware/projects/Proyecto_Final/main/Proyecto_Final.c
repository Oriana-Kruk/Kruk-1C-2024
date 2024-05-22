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

#include <stdbool.h>
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



/*==================[macros and definitions]=================================*/
Si7007_config my_sensor;
/*==================[internal data definition]===============================*/

 /** @fn  typedef struct
  * @brief estrutura que representa el pin GPIO que está conectado al sensor de línea
 */
typedef struct {
	gpio_t line_sensor_pin;  // Pin asociado al sensor de línea
} LineSensor_config;


 //se crea una instancia de LineSensor_config llamada lineSensorConfig y inicializándola con el pin
LineSensor_config lineSensorConfig = {
    .line_sensor_pin = GPIO_4 // 
};

volatile uint32_t lineCount = 0;  // Contador de líneas detectadas
TaskHandle_t xTaskToNotify = NULL;  // Handle de la tarea que será notificada

/*==================[internal functions declaration]=========================*/

 /** @fn  float getTemperature(void)
  * @brief La función "getTemperature" llama a "Si7007MeasureTemperature", que es una función del driver del sensor 
  * que realiza la medición de la temperatura. El valor de la temperatura medido se almacena en la variable temperature.
 */
float getTemperature(void);

 /** @fn  getHumidity(void)
  * @brief La función "getHumidity" llama a "Si7007MeasureHumidity", que es una función del driver del sensor 
  * que realiza la medición de la humedad. El valor de la humedad se almacena en la variable humidity.
 */
float getHumidity(void);

 /** @fn void initLineSensor(LineSensor_config *config)
  * @brief Esta función inicializa el sensor de línea TCRT5000 configurando el pin especificado en el parámetro config como entrada.
  * Además, añade un manejador de interrupción para detectar cambios en el estado del sensor de línea
  * @param LineSensor_config *config
 */
void initLineSensor(LineSensor_config *config);

 /** @fn  bool detectLine(void)
  * @brief Esta función lee el valor actual del pin del sensor de línea y devuelve true si el sensor detecta una línea (valor bajo) 
  * o false si no la detecta (valor alto).
 */
bool detectLine(void);

 /** @fn void timerCallback(void *param)
  * @brief Esta función es llamada periódicamente por el timer. Su propósito principal es calcular la frecuencia de detección de líneas
  * basándose en el valor del contador lineCount, el cual se reinicia en cada llamada. La frecuencia se puede convertir a una velocidad
  * del viento utilizando un factor de conversión.
  * @param void *param
 */
void timerCallback(void *param);

 /** @fn void IRAM_ATTR lineDetectionISR(void* arg)
  * @brief Esta función es una rutina de servicio de interrupción (ISR) que se ejecuta cuando el sensor de línea detecta un cambio
  * de estado. Incrementa el contador lineCount y notifica a una tarea de FreeRTOS que la interrupción ha ocurrido. 
  * @param void* arg
 */
void IRAM_ATTR lineDetectionISR(void* arg);

/*==================[external functions definition]==========================*/


float getTemperature(void) {
    // Medir la temperatura
    float temperature = Si7007MeasureTemperature();
    return temperature;
}

float getHumidity(void) {
    // Medir la humedad
    float humidity = Si7007MeasureHumidity();
    return humidity;
}

void initLineSensor(LineSensor_config *config) {
    // Configurar el pin del sensor de línea como entrada
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->line_sensor_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf);

    // Configurar la interrupción para detección de línea
    //gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(config->line_sensor_pin, lineDetectionISR, NULL);
}

bool detectLine(void) {
    // Leer el valor del pin del sensor de línea
    return GPIORead(lineSensorConfig.line_sensor_pin);
}

void IRAM_ATTR lineDetectionISR(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    lineCount++;

    // Notificar a la tarea que la interrupción ha ocurrido
    vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);

    // Si una tarea de mayor prioridad fue despertada por la interrupción, realizar un cambio de contexto
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void timerCallback(void *param) {
    // Obtener la frecuencia de detección de líneas (en Hz)
    uint32_t frequency = lineCount; // Número de líneas detectadas en el período del timer
    lineCount = 0;  // Reiniciar el contador de líneas detectadas

    // Calcular la velocidad del viento (esto es un ejemplo y puede necesitar ajustes)
    float windSpeed = frequency * FACTOR_CONVERSION; // FACTOR_CONVERSION debe ser determinado experimentalmente
}
void vLineDetectionTask(void *pvParameters) {
   
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }


void app_main(void){
	
    Si7007Init(&my_sensor);  // Inicializar el sensor Si7007

	// Crear la tarea que será notificada por la interrupción
    xTaskCreate(vLineDetectionTask, "Line Detection Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xTaskToNotify);
	// Inicializar el sensor de línea
    initLineSensor(&lineSensorConfig);
	 // Iniciar el scheduler de FreeRTOS
    vTaskStartScheduler();


   // configura el sensor Si7007
	Si7007_config my_sensor = {
		.select = GPIO_1,
		.PWM_1 = GPIO_2,
		.PWM_2 = GPIO_3,
	};

  // Configurar e iniciar el timer
    timer_config_t timerConfig = {
        .timer = TIMER_A,
        .period = 1000000,  // Periodo en microsegundos (1 segundo)
        .func_p = timerCallback,
        .param_p = NULL
    };
    TimerInit(&timerConfig);
    TimerStart(TIMER_A);

	 

}

/*==================[end of file]============================================*/