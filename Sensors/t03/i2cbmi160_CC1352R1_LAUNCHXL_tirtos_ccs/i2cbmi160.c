#include <stdint.h>
#include <stddef.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/display/Display.h>
#include <ti/sail/bmi160/bmi160.h>
#include "bmi160_support.h"
#include "ti_drivers_config.h"

I2C_Handle      i2c;
I2C_Params      i2cParams;

Display_Handle display;

void *mainThread(void *arg0) {

    GPIO_init();
    I2C_init();

    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        while (1);
    }

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cParams.transferMode = I2C_MODE_BLOCKING;
    i2cParams.transferCallbackFxn = NULL;

    i2c = I2C_open(CONFIG_I2C_BMI, &i2cParams);

    if (i2c == NULL) {
        Display_print0(display, 0, 0, "Error Initializing I2C\n");
    }

    bmi160_initialize_sensor(i2c);

    return (0);
}
