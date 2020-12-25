#include <i2copt3001_header.h>

void *mainThread(void *arg0) {

    I2C_Handle      i2cHandle;
    I2C_Params      i2cParams;
    pthread_t alertTask;
    pthread_attr_t       pAttrs;
    int             retc;

    GPIO_init();
    I2C_init();
    OPT3001_init();

    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        while (1);
    }

    GPIO_write(CONFIG_LED_0_GPIO, CONFIG_GPIO_LED_ON);
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cHandle = I2C_open(CONFIG_I2C_OPT, &i2cParams);
    if (i2cHandle == NULL) {
        Display_print0(display, 0, 0, "Error Initializing I2C\n");
        while (1);
    }
	
    if(0 != sem_init(&opt3001Sem,0,0)) {
        Display_print0(display, 0, 0, "opt3001Sem Semaphore creation failed");
        while (1);
    }

    pthread_attr_init(&pAttrs);
    pthread_attr_setstacksize(&pAttrs, OPT_TASK_STACK_SIZE);
    retc = pthread_create(&alertTask, &pAttrs, opt3001InterruptTask, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        Display_print0(display, 0, 0, "Alert Task creation failed");
        while (1);
    }

    OPT3001_Params_init(&opt3001Params);

    opt3001Params.callback = opt3001Callback;

    opt3001Handle = OPT3001_open(CONFIG_OPT3001_LIGHT, i2cHandle, &opt3001Params);

    if(opt3001Handle == NULL) {
        Display_print0(display, 0, 0, "OPT3001 Open Failed!");
        while(1);
    }
    sleep(1);

    /* Enable interrupts from OPT3001 */
    OPT3001_enableInterrupt(opt3001Handle);
    while(1) {
        if (!OPT3001_getLux(opt3001Handle, &lux))
            Display_print0(display, 0, 0, "OPT3001 sensor read failed");
        Display_print1(display, 0, 0, "Lux: %f", lux);
        sleep(10);
    }
}
