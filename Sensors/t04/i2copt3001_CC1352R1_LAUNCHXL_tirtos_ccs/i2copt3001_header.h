#include <stdint.h>
#include <stddef.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/display/Display.h>

/* Module Header */
#include <ti/sail/opt3001/opt3001.h>

/* Driver configuration */
#include "ti_drivers_config.h"

static Display_Handle display;

#define SAMPLE_TIME      1        /*In seconds*/
#define HIGH_LIMIT       25000.0F
#define LOW_LIMIT        100.0F

#define OPT_TASK_STACK_SIZE   768

#define CONFIG_OPT3001_LIGHT 0
#define CONFIG_OPT3001_COUNT 1

OPT3001_Handle opt3001Handle = NULL;
OPT3001_Params opt3001Params;

/* Please check <ti/sail/opt3001/opt3001.h> file to know more about OPT3001_HWAttrs and OPT3001_Config structures */
OPT3001_Object OPT3001_object[CONFIG_OPT3001_COUNT];

const OPT3001_HWAttrs OPT3001_hwAttrs[CONFIG_OPT3001_COUNT] = {
    {
#ifdef CONFIG_I2C_OPT_BOOSTXL_SENSORS_OPT3001_ADDR // BOOSTXL-SENSORS
        .slaveAddress = OPT3001_SA4,
#else // BOOSTXL-BASSENSORS
        .slaveAddress = OPT3001_SA1,
#endif
        .gpioIndex = CONFIG_GPIO_OPT3001_INT,
    },
};

const OPT3001_Config OPT3001_config[] = {
    {
        .hwAttrs = &OPT3001_hwAttrs[0],
        .object  = &OPT3001_object[0],
    },
    {NULL, NULL},
};

/* Global lux values which may be accessed from GUI Composer App */
float lux;

/* Global sample rate which may be accessed and set from GUI Composer App */
volatile uint16_t sampleTime;

sem_t opt3001Sem;

/*
 *  ======== opt3001Callback ========
 *  When a fault condition is met on the opt3001 hardware, the INT pin is
 *  asserted generating an interrupt. This callback function serves as an ISR
 *  for a single opt3001 sensor.
 */
void opt3001Callback(uint_least8_t index) {

    sem_post(&opt3001Sem);
}

/*
 *  ======== opt3001InterruptTask ========
 *  This task is unblocked when the INT pin is asserted and generates an
 *  interrupt. When the OPT3001 is in LATCH mode, the configuration register
 *  must be read to the clear the INT pin.
 */
void *opt3001InterruptTask(void *arg0)
{
    uint16_t data;

    while (1)
    {

        /* Pend on the semaphore, opt3001Sem */
        if (0 == sem_wait(&opt3001Sem)) {

            /* Read config register, resetting the INT pin */
            OPT3001_readRegister(opt3001Handle, &data, OPT3001_CONFIG);

            if (data & OPT3001_FL) {
                Display_print0(display, 0, 0, "ALERT INT: Lux Low\n");
            }

            if (data & OPT3001_FH) {
                Display_print0(display, 0, 0, "ALERT INT: Lux High\n");
            }
        }
    }
}
