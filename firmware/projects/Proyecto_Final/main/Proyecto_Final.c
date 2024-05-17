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

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void initSensor(void);
float getTemperature(void);
float getHumidity(void);

/*==================[external functions definition]==========================*/

void initSensor(void) {
    Si7007Init(&sensorConfig);  // Inicializar el sensor Si7007
}

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

void app_main(void){
    
    initSensor(); // Inicializar el sensor
    
    // Recolectar datos (aquí se podrían almacenar o procesar)
    //float temperature = getTemperature();
    //float humidity = getHumidity();


}

/*==================[end of file]============================================*/