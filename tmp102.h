#pragma once

#include <stdio.h>
#include <avr/io.h>
#include "i2c.h"

#define COMPLETE_OR_FAIL if(i2cWaitForComplete()){return 0x0FFF;}

uint16_t tmp102_configure(uint8_t address){
	i2cSendStart();	
    i2cSendByte(address);
	COMPLETE_OR_FAIL; i2cSendByte(0x01);
	COMPLETE_OR_FAIL; i2cSendByte(0b01100000);
	COMPLETE_OR_FAIL; i2cSendByte(0b10110000);
	COMPLETE_OR_FAIL; i2cSendStop();
	return 0;
}

int16_t tmp102_readraw(uint8_t address){
	uint16_t msb, lsb;
	i2cSendStart();	
    COMPLETE_OR_FAIL;i2cSendByte(address);
	COMPLETE_OR_FAIL;i2cSendByte(0);
	COMPLETE_OR_FAIL;i2cSendStart();
	i2cSendByte(address|1);
	COMPLETE_OR_FAIL;i2cReceiveByte(-1);
	COMPLETE_OR_FAIL;msb = i2cGetReceivedByte();
	COMPLETE_OR_FAIL;i2cReceiveByte(0);
	COMPLETE_OR_FAIL;lsb = i2cGetReceivedByte();
	COMPLETE_OR_FAIL;i2cSendStop();
	int16_t raw_temp = (msb<<5)|(lsb>>3);
	if(raw_temp&0x1000){
		raw_temp = -(((~raw_temp)&0xFFF) + 1);
	}
	return raw_temp;
}
