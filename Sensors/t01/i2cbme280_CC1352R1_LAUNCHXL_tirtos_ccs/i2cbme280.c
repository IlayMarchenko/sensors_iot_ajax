#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/sail/bme280/bme280.h>
#include "ti_drivers_config.h"

s32 Temperature = 0;
u32 Pressure    = 0;
u32 Humidity    = 0;

I2C_Handle      i2c;
I2C_Params      i2cParams;

UART_Handle uart;
UART_Params uartParams;

char msg[65];

extern s32 bme280_data_readout_template(I2C_Handle i2cHndl);

void *mainThread(void *arg0) {

    I2C_init();
    GPIO_init();
    UART_init();

    UART_Params_init(&uartParams);

    uart = UART_open(CONFIG_UART_0, &uartParams);

    if (uart == NULL) {
        while (1);
    }

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_BME, &i2cParams);
    if (i2c == NULL) {
        GPIO_write(RED_LED, CONFIG_GPIO_LED_ON);
        UART_write(uart, "Error Initializing I2C\n\r", sizeof("Error Initializing I2C\n\r"));
    }
    else
        GPIO_write(GREEN_LED, CONFIG_GPIO_LED_ON);

    /* Initialize the BME Sensor */
    if(BME280_INIT_VALUE != bme280_data_readout_template(i2c)) {
        GPIO_write(RED_LED, CONFIG_GPIO_LED_ON);
        GPIO_write(GREEN_LED, CONFIG_GPIO_LED_OFF);
        UART_write(uart, "Error Initializing bme280\n\r", sizeof("Error Initializing bme280\n\r"));
    }

    bme280_set_power_mode(BME280_NORMAL_MODE);

    while(1) {
        if(BME280_INIT_VALUE == bme280_read_pressure_temperature_humidity(&Pressure, &Temperature, &Humidity)) {
            sprintf(&msg, "%d KPa(Pressure), %d DegC(Temperature), %d %RH(Humidity)\n\r", Pressure/1000, Temperature/100, Humidity/1000);
            UART_write(uart, msg, sizeof(msg));
        }
        else {
            GPIO_write(RED_LED, CONFIG_GPIO_LED_ON);
            UART_write(uart, "Error reading from the bme280 sensor\n\r", sizeof("Error reading from the bme280 sensor\n\r"));
        }

        sleep(10);
    }
}

