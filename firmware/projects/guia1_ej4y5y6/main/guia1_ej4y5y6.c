/*! @mainpage guia1_ej4y5y6
 *
 * @section genDesc General Description
 *
 * El programa define una función convertToBcdArray que convierte un dato de 32 bits a formato BCD y almacena cada dígito en un arreglo.
 * Luego, se define una función que cambia el estado de los pines GPIO según los bits de un dígito BCD ingresado, utilizando una 
 * estructura gpioConf_t para configurar los pines. Finalmente, se crea una función para mostrar un dato en un display LCD
 * utilizando dos vectores de estructuras gpioConf_t para mapear los pines GPIO correspondientes a los dígitos del LCD y configurar 
 * la visualización del dato recibido.
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
 * | 22/03/2024 | Document creation		                         |
 *
 * @author Kruk Oriana
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

 /** @struct typedef struct gpioConf_t
  * @brief almacena el numero de pin y la direccion de GPIO
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

 /** @fn int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
  * @brief convertir el dato a BCD
  * @param .uint32_t data, uint8_t digits, uint8_t *bcd_number
  * @return
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);

 /** @fn  void cambiarEstadoGPIO(uint8_t bcd, gpioConf_t *gpioArray);
  * @brief toma un número BCD y un array de configuraciones de GPIO
  * @param .uint8_t bcd, gpioConf_t *gpioArray
  * @return
 */
void cambiarEstadoGPIO(uint8_t bcd, gpioConf_t *gpioArray);

 /** @fn  void mostrarDatoLCD(uint32_t data, uint8_t digits, gpioConf_t *gpioArray, gpioConf_t *gpioMap, uint8_t *bcd_number)
  * @brief Convierte el número data en un array de dígitos BCD, luego recorre cada dígito BCD y 
  *        realiza acciones en los pines GPIO según la configuración proporcionada en los arrays gpioArray y gpioMap. muestra el dato por display
  * @param .uint32_t data, uint8_t digits, gpioConf_t *gpioArray, gpioConf_t *gpioMap, uint8_t *bcd_number
  * @return
 */
void mostrarDatoLCD(uint32_t data, uint8_t digits, gpioConf_t *gpioArray, gpioConf_t *gpioMap, uint8_t *bcd_number);

/*==================[external functions definition]==========================*/

//ej(4)
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number) {
    // Verificar si la cantidad de dígitos es válida
    if (digits <= 0 || digits > 10) {
        return -1; // Cantidad de dígitos no válida
    }
    // Convertir el dato a BCD y almacenar los dígitos en el arreglo
    for (int i = digits - 1; i >= 0; i--) {
        bcd_number[i] = data % 10; // Obtener el dígito menos significativo
        data /= 10; // Eliminar el dígito menos significativo
    }
    return 0; // Conversión exitosa
}
//ej(5)
//defino la función cambiarEstadoGPIO que toma un número BCD y un array de configuraciones de GPIO
 void cambiarEstadoGPIO(uint8_t bcd, gpioConf_t *gpioArray)
 {
    for (int i = 0; i < 4; i++) // recorre el array de configuraciones GPIO, que tiene 4 elementos
    {
        if((bcd>>i)&1) //verifica si el bit correspondiente en el número BCD es 1
        {
            GPIOOn(gpioArray[i].pin); //Si el bit es 1, enciende el pin GPIO utilizando la función GPIOOn
        }
        else
        GPIOOff(gpioArray[i].pin); //Si el bit es 0, apaga el pin GPIO utilizando la función GPIOOff
    }  
 }
 //ej(6)
 /*esta función toma un número 'data', la cantidad de dígitos 'digits', dos arrays de configuraciones GPIO (gpioArray y gpioMap), 
 y un array de dígitos BCD (bcd_number). Convierte el número data en un array de dígitos BCD, luego recorre cada dígito BCD y 
 realiza acciones en los pines GPIO según la configuración proporcionada en los arrays gpioArray y gpioMap.*/

 void mostrarDatoLCD(uint32_t data, uint8_t digits, gpioConf_t *gpioArray, gpioConf_t *gpioMap, uint8_t *bcd_number)
 {
    convertToBcdArray(data,digits,bcd_number); //convierte el número data en un array de dígitos BCD utilizando la función convertToBcdArray
    for (int i = 0; i < digits; i++)//recorro los dígitos del array BCD
    {
        cambiarEstadoGPIO(bcd_number[i],gpioArray);//// Para cada dígito BCD, cambia el estado de los pines GPIO utilizando la función cambiarEstadoGPIO
        GPIOOn(gpioMap[i].pin);//Enciende el pin GPIO correspondiente al dígito en el mapa de GPIO utilizando la función GPIOOn
        GPIOOff(gpioMap[i].pin);//Apaga el pin GPIO correspondiente al dígito en el mapa de GPIO utilizando la función GPIOOff
    }   
 }

void app_main(void){
	uint8_t digit=3;
    uint32_t data = 123;
    uint8_t bcd_number[digit]; // Tamaño suficiente para almacenar los 3 dígitos de 123 en BCD
    convertToBcdArray(data, digit, bcd_number); //llamo a la funcion
	
  /*  for(int i=0; i<digit; i++)
    {
         printf("%u",bcd_number[i]);
    }*/
   
    gpioConf_t gpioArray[4]= //Defino un array de configuraciones GPIO llamado gpioArray con 4 elementos
    {
        {GPIO_20, GPIO_OUTPUT}, //Configuro el cada pin como salida porque va a la pantalla.
        {GPIO_21, GPIO_OUTPUT},
        {GPIO_22, GPIO_OUTPUT},
        {GPIO_23, GPIO_OUTPUT},
    };
//INICIALIZO GPIO de cambio de estado
    for (int i = 0; i < 4; i++)
    {
        GPIOInit(gpioArray[i].pin, gpioArray[i].dir);
    }
    /*gpioMap[i].pin representa el pin GPIO asociado al elemento actual del array gpioMap.
    gpioMap[i].dir representa la dirección (entrada o salida) que se le está asignando al pin GPIO */

    gpioConf_t gpioMap[3]=// Defino un array de configuraciones GPIO llamado gpioMap con 3 elementos
    {
       {GPIO_19, GPIO_OUTPUT},//Configuro cada pin como salida para mostrar datos en el LCD
       {GPIO_18, GPIO_OUTPUT},
       {GPIO_9, GPIO_OUTPUT}, 
    };
//inicializo GPIO de muestra dato lcd
    for (int i = 0; i < 3; i++)
    {
        GPIOInit(gpioMap[i].pin, gpioMap[i].dir);
    };
}
/*==================[end of file]============================================*/