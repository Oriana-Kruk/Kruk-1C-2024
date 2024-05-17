#ifndef SI7007_H_
#define SI7007_H_

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros]=================================================*/
#define V_REF 3.3                /**< Tensión de referencia */
#define TOTAL_BITS 1024          /**< Cantidad total de bits */

/*==================[typedef]================================================*/
typedef struct {                /*!< Si7007 Inputs config struct */
    gpio_t select;              /*!< Determines the output on each pin */
    uint8_t PWM_1;              /*!< Channel connected to the PWM1 output pin on the device */
    uint8_t PWM_2;              /*!< Channel connected to the PWM2 output pin on the device */
} Si7007_config;

/*==================[external functions declaration]=========================*/
bool Si7007Init(Si7007_config *pins);
float Si7007MeasureTemperature(void);
float Si7007MeasureHumidity(void);
bool Si7007Deinit(Si7007_config *pins);

#endif /* #ifndef SI7007_H */