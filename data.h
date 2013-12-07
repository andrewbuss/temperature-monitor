#pragma once

#include "eeprom.h"

uint8_t cache[256];
uint8_t pn = 0;
uint8_t pi = 0;

inline void data_skip_back(uint16_t* address, uint16_t n){
	while(n>0){
		*address -= 6;
		if((*address)%256 == 250) *address -= 16;
		n--;
	}
}

uint16_t data_get_live(uint16_t address){
	if(address/256 == pn) return (cache[address%256]<<8)|cache[address%256+1];
	else return (eeprom_readbyte(address)<<8)|eeprom_readbyte(address+1);
}

void data_flush_cache(void){
	eeprom_writepage(pn,cache);
}

void data_reset_cache(void){
	for(uint16_t i=0;i<128;i++){
		cache[2*i] = 0x0F;
		cache[2*i+1] = 0xFF;
	}
}

void data_load_page(uint8_t page){
	uint8_t i=0;
	printf_P(PSTR("Page 0x%02X:\n"),page);
	while(1){
		eeprom_reset();
		uint16_t address = (page<<8)|i;
		cache[i] = eeprom_readbyte(address);
		printf_P(PSTR("%02X "),cache[i]);
		i++;
		if(i%16 == 0) printf_P(PSTR("\n"));
		if(i==0) break;
	}
}

void data_log(int16_t a, int16_t b, int16_t c){
	printf_P(PSTR("logging to cache for page 0x%02X, starting at 0x%02X\n"),pn,pi);
	cache[pi++] = (((uint16_t)a)>>8)&0xFF;
	cache[pi++] = ((uint16_t)a)&0xFF;
	cache[pi++] = (((uint16_t)b)>>8)&0xFF;
	cache[pi++] = ((uint16_t)b)&0xFF;
	cache[pi++] = (((uint16_t)c)>>8)&0xFF;
	cache[pi++] = ((uint16_t)c)&0xFF;
	if(pi%120 == 0){
		printf_P(PSTR("saving cached page to EEPROM\n"));
		data_flush_cache();
	}
	if(pi==240){
		pi=0;
		pn++;
		printf_P(PSTR("beginning on page %02X\nresetting page\nsaving reset page to EEPROM\n"),pn);
		data_reset_cache();
		data_flush_cache();
	}
}