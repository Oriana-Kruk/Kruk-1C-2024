/*==================[inclusions]=============================================*/
#include "si7007.h"
#include <stdint.h>
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
analog_input_config_t temp_config;
analog_input_config_t hum_config;
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions declaration]=========================*/

bool Si7007Init(Si7007_config *pins){
    
	GPIOInit(pins->select, GPIO_OUTPUT);
	GPIOOn(pins->select); //Lo pongo en 1 para que PWM 2 sea temperatura y PWM 1 humedad.

	temp_config.input = pins->PWM_2;
	temp_config.mode = ADC_SINGLE;

	hum_config.input = pins->PWM_1;
	hum_config.mode = ADC_SINGLE;

    return true; 
}

float Si7007MeasureTemperature(void) {
    uint16_t value;
    float conversion = 0;
    float temperature = 0;
    float valor = 0;

    AnalogInputInit(&temp_config);
    AnalogInputReadSingle(temp_config.input, &value); 
    conversion = value * V_REF / TOTAL_BITS;
    valor = conversion / V_REF;
    temperature = -46.85 + (valor * 175.71); //  según la hoja de datos.

    return temperature;
}
float Si7007MeasureHumidity(void) {
    uint16_t value;
    float conversion = 0;
    float humidity = 0;
    float valor = 0;

    AnalogInputInit(&hum_config);
    AnalogInputReadSingle(hum_config.input, &value); 
    
    conversion = value * V_REF / TOTAL_BITS;
    valor = conversion / V_REF;
    humidity = -6 + (valor * 125); //según la hoja de datos.

    return humidity;
}

bool Si7007Deinit(Si7007_config *pins){
return true; 
}

/*==================[end of file]============================================*/