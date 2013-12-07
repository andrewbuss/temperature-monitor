#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "common.h"

#define EEPROM_SDA_PIN 2
#define EEPROM_SCL_PIN 1
#define EEPROM_PORT PORTB
#define EEPROM_DDR DDRB
#define EEPROM_DELAY //for(unsigned int i=0;i<5;i++) asm volatile ("nop");
#define EEPROM_CLOCK_HIGH SET_BIT(EEPROM_PORT,EEPROM_SCL_PIN);
#define EEPROM_CLOCK_LOW CLR_BIT(EEPROM_PORT,EEPROM_SCL_PIN);
#define EEPROM_DATA_HIGH SET_BIT(EEPROM_PORT,EEPROM_SDA_PIN);
#define EEPROM_DATA_LOW CLR_BIT(EEPROM_PORT,EEPROM_SDA_PIN);
#define EEPROM_DATA_IN CLR_BIT(EEPROM_DDR,EEPROM_SDA_PIN); EEPROM_DATA_LOW;
#define EEPROM_DATA_OUT SET_BIT(EEPROM_DDR,EEPROM_SDA_PIN); EEPROM_DATA_HIGH;

void eeprom_start(void){
	EEPROM_DATA_OUT;
	EEPROM_DATA_HIGH;
	EEPROM_DELAY;
	EEPROM_CLOCK_HIGH;
	EEPROM_DELAY;
	EEPROM_DATA_LOW;
	EEPROM_DELAY;
	EEPROM_CLOCK_LOW;
	EEPROM_DELAY;
	EEPROM_DATA_IN;
	EEPROM_DATA_HIGH;
}

void eeprom_stop(void){
	EEPROM_DATA_OUT;
	EEPROM_DATA_LOW;
	EEPROM_DELAY;
	EEPROM_CLOCK_HIGH;
	EEPROM_DELAY;
	EEPROM_DATA_HIGH;
	EEPROM_DELAY;
	EEPROM_CLOCK_LOW;
	EEPROM_DELAY;
	EEPROM_DATA_IN;
	EEPROM_DATA_HIGH;
}

void eeprom_reset(void){
	eeprom_start();
	EEPROM_DATA_OUT;
	EEPROM_DATA_HIGH;
	EEPROM_DELAY;
	for(int i=0;i<9;i++){
		EEPROM_CLOCK_HIGH;
		EEPROM_DELAY;
		EEPROM_CLOCK_LOW;
		EEPROM_DELAY;
	}
	eeprom_start();
	eeprom_stop();
}

void eeprom_sendbit(uint8_t bit){
	EEPROM_DATA_OUT;
	if(bit){EEPROM_DATA_HIGH;}
	else{EEPROM_DATA_LOW;}
	EEPROM_DELAY;
	EEPROM_CLOCK_HIGH;
	EEPROM_DELAY;
	EEPROM_CLOCK_LOW;
	EEPROM_DATA_IN;
	EEPROM_DELAY;
	EEPROM_DATA_HIGH;
}

uint8_t eeprom_receivebit(void){
	EEPROM_DATA_IN;
	EEPROM_DATA_HIGH;
	EEPROM_DELAY;
	EEPROM_CLOCK_HIGH;
	EEPROM_DELAY;
	uint8_t bit = (PINB>>EEPROM_SDA_PIN)&1;
	EEPROM_CLOCK_LOW;
	EEPROM_DELAY;
	EEPROM_DATA_HIGH;
	return bit;
}

void eeprom_ack(void){
	EEPROM_DATA_OUT;
	EEPROM_DATA_LOW;
	EEPROM_DELAY;
	EEPROM_CLOCK_HIGH;
	EEPROM_DELAY;
	EEPROM_CLOCK_LOW;
	EEPROM_DELAY;
	EEPROM_DATA_HIGH;
	EEPROM_DATA_IN;
}

void eeprom_nack(void){
	EEPROM_DATA_OUT;
	EEPROM_DATA_HIGH;
	EEPROM_DELAY;
	EEPROM_CLOCK_HIGH;
	EEPROM_DELAY;
	EEPROM_CLOCK_LOW;
	EEPROM_DELAY;
	EEPROM_DATA_HIGH;
	EEPROM_DATA_IN;
}

void eeprom_sendbyte(uint8_t byte){
	for(int i=0;i<8;i++){
		eeprom_sendbit(byte&128);
		byte<<=1;
	}
}

// void eeprom_writebyte(uint16_t address, uint8_t byte){
// 	cli();
// 	eeprom_reset();
// 	eeprom_start();
// 	eeprom_sendbyte(0b10100000);
// 	if(eeprom_receivebit()) //printf_P(PSTR("Sent address not acknowledged\n"))
// 	;
// 	eeprom_sendbyte((address>>8)&0xFF);
// 	if(eeprom_receivebit()) //printf_P(PSTR("Sent MSB not acknowledged\n"))
// 	;
// 	eeprom_sendbyte(address & 0xFF);
// 	if(eeprom_receivebit()) //printf_P(PSTR("Sent LSB not acknowledged\n"))
// 	;
// 	eeprom_sendbyte(byte);
// 	if(eeprom_receivebit()) //printf_P(PSTR("Sent data byte not acknowledged\n"))
// 	;
// 	eeprom_stop();
// 	sei();
// }

uint8_t eeprom_receivebyte(void){
	uint8_t byte = 0;
	for(int i=0;i<8;i++){
		byte = (byte<<1) | eeprom_receivebit();
	}
	return byte;
}
uint8_t eeprom_readbyte(uint16_t address){
	cli();
	eeprom_reset();
	eeprom_start();
	eeprom_sendbyte(0b10100000);
	eeprom_receivebit();
	eeprom_sendbyte((address>>8)&0xFF);
	eeprom_receivebit();
	eeprom_sendbyte(address & 0xFF);
	eeprom_receivebit();
	eeprom_start();
	eeprom_sendbyte(0b10100001);
	eeprom_receivebit();
	uint8_t byte = eeprom_receivebyte();
	eeprom_receivebit();
	eeprom_nack();
	eeprom_stop();
	sei();
	return byte;
}

void eeprom_waituntilready(void){
	check:
	eeprom_start();
	eeprom_sendbyte(0b10100000);
	printf_P(PSTR("."));
	if(eeprom_receivebit()) goto check;
	eeprom_reset();
	return;
}

void eeprom_writepage(uint8_t pagenum, uint8_t* page){
	cli();
	eeprom_reset();
	eeprom_start();
	eeprom_sendbyte(0b10100000);
	if(eeprom_receivebit()) //printf_P(PSTR("Sent address not acknowledged\n"))
	;
	eeprom_sendbyte(pagenum);
	if(eeprom_receivebit()) //printf_P(PSTR("Sent MSB not acknowledged\n"))
	;
	eeprom_sendbyte(0);
	if(eeprom_receivebit()) //printf_P(PSTR("Sent LSB not acknowledged\n"))
	;
	uint8_t i=0;
	while(1){
		eeprom_sendbyte(*page);
		if(eeprom_receivebit()) //printf_P(PSTR("Sent data byte not acknowledged\n"))
		;
		i++;
		page++;
		if(i==0) break;
	}
	eeprom_stop();
	sei();
	printf_P(PSTR("Waiting until EEPROM is ready"));
	eeprom_waituntilready();
	printf_P(PSTR(" - ready!\n"));
	page -= 256;
	i=0;
	while(1){
		if(eeprom_readbyte((pagenum<<8)|i) != *page) printf("Mismatch at %04X\n", (pagenum<<8)|i);
		page++;
		i++;
		if(i==0) break;
	}
	
}

// void eeprom_readpage(uint8_t pagenum, uint8_t* page){
// 	cli();
// 	eeprom_reset();
// 	eeprom_start();
// 	eeprom_sendbyte(0b10100000);
// 	if(eeprom_receivebit()) //printf_P(PSTR("Sent address not acknowledged\n"))
// 	;
// 	eeprom_sendbyte(pagenum);
// 	if(eeprom_receivebit()) //printf_P(PSTR("Sent MSB not acknowledged\n"))
// 	;
// 	eeprom_sendbyte(0);
// 	if(eeprom_receivebit()) //printf_P(PSTR("Sent LSB not acknowledged\n"))
// 	;
// 	eeprom_start();
// 	eeprom_sendbyte(0b10100001);
// 	if(eeprom_receivebit()) //printf_P(PSTR("Sent address not acknowledged\n"))
// 	;
// 	uint8_t i=0;
// 	printf_P(PSTR("\n"));
// 	while(1){
// 		*page = eeprom_receivebyte();
// 		//printf_P(PSTR("%02X"),*page);
// 		eeprom_ack();
// 		i++;
// 		page++;
// 		if(i==0) break;
// 	}
// 	printf_P(PSTR("\n"));
// 	eeprom_nack();
// 	eeprom_stop();
// 	sei();
// }