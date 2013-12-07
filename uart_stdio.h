#include <avr/io.h>
#include <stdio.h>
#include "uart.h"

uint8_t uart_stdio_putchar(char c, FILE *stream){
    uart_putc(c);
    return 0;
}

uint8_t uart_stdio_getchar(void){
	uint16_t c;
	again:
	c = uart_getc();
	if(c & UART_NO_DATA) goto again;
	return c&255;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_stdio_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_stdio_getchar, _FDEV_SETUP_READ);

void uart_stdio_init(void){
	stdout = &uart_output;
    stdin = &uart_input;
}