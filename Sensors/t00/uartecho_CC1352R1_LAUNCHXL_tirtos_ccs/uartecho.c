#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include "ti_drivers_config.h"

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0) {
    char        input;
    char        input_string [100];
    uint8_t     input_counter;
    UART_Handle uart;
    UART_Params uartParams;

    GPIO_init();
    UART_init();

    UART_Params_init(&uartParams);

    uart = UART_open(CONFIG_UART_0, &uartParams);

    if (uart == NULL) {
        GPIO_write(LED_RED, CONFIG_GPIO_LED_ON);
        while (1);
    }

    GPIO_write(LED_GREEN, CONFIG_GPIO_LED_ON);

    for (input_counter = 0; input_counter < 100; input_counter++) {
        input_string[input_counter] = '\0';
    }
    input_counter = 0;

    while (1) {
        UART_read(uart, &input, 1);
        if (input == 13) {
            UART_write(uart, "\r\n", 2);
            if (input_counter != 0) {
                input_counter = 0;
                UART_write(uart, input_string, sizeof(input_string));
            }
            UART_write(uart, "\r\n", 2);
            uint8_t i;
            for (i = 0; i < 100; i++) {
                input_string[i] = '\0';
            }
        }
        else {
            UART_write(uart, &input, 1);
            input_string[input_counter++] = input;
        }
    }
}

