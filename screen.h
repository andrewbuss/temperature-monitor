#pragma once

#include <avr/io.h>
#include <math.h>
#include "common.h"
#include "xbm.h"

#define SCREEN_PORT PORTD
#define SCREEN_CLOCK_PIN 5
#define SCREEN_DATA_PIN 6
#define SCREEN_REG_PIN 2
#define SCREEN_RESET_PIN 3
#define SCREEN_CS_PIN 4
#define SCREEN_CLOCK_HIGH SET_BIT(SCREEN_PORT, SCREEN_CLOCK_PIN);
#define SCREEN_CLOCK_LOW CLR_BIT(SCREEN_PORT, SCREEN_CLOCK_PIN);
#define SCREEN_DATA_HIGH SET_BIT(SCREEN_PORT, SCREEN_DATA_PIN);
#define SCREEN_DATA_LOW CLR_BIT(SCREEN_PORT,SCREEN_DATA_PIN);
#define SCREEN_CS_HIGH //SET_BIT(SCREEN_PORT, SCREEN_CS_PIN);
#define SCREEN_CS_LOW CLR_BIT(SCREEN_PORT,SCREEN_CS_PIN);
#define SCREEN_RESET CLR_BIT(SCREEN_PORT,SCREEN_RESET_PIN); SET_BIT(SCREEN_PORT,SCREEN_RESET_PIN);
#define SCREEN_INST_REG CLR_BIT(SCREEN_PORT,SCREEN_REG_PIN);
#define SCREEN_DATA_REG SET_BIT(SCREEN_PORT,SCREEN_REG_PIN);

uint8_t screen_buffer[8][128];
const uint8_t round_fractionals[16] = {0,6,12,19,25,31,37,44,50,56,62,69,75,81,87,94};
const uint8_t letters_bits[] = {
   0xdf, 0xf3, 0x3d, 0xdf, 0xf7, 0x45, 0x1f, 0x14, 0x05, 0x51, 0xf4, 0x3d,
   0xdf, 0xf3, 0x7d, 0x51, 0x14, 0x45, 0xd1, 0x07, 0x51, 0x14, 0x44, 0x41,
   0x10, 0x44, 0x04, 0x94, 0x04, 0xdb, 0x14, 0x45, 0x51, 0x14, 0x10, 0x51,
   0x14, 0x29, 0x11, 0x02, 0xdf, 0x17, 0x44, 0xc7, 0xd1, 0x7d, 0x04, 0x74,
   0x04, 0x55, 0x15, 0x7d, 0xd1, 0xf7, 0x11, 0x51, 0x54, 0x11, 0x1f, 0x01,
   0x51, 0x14, 0x44, 0x41, 0x10, 0x45, 0x44, 0x94, 0x04, 0x51, 0x16, 0x05,
   0x59, 0x02, 0x11, 0x91, 0xb2, 0x29, 0x84, 0x00, 0xd1, 0xf3, 0x3d, 0x5f,
   0xf0, 0x45, 0xdf, 0x17, 0x7d, 0x51, 0xf4, 0x05, 0x5f, 0xf4, 0x11, 0x1f,
   0x11, 0x45, 0xc4, 0x07 };

const uint8_t  numerals_bits[] = {
   0xdf, 0xbe, 0x2f, 0xfa, 0xbe, 0xef, 0xfb, 0x00, 0x91, 0x20, 0x28, 0x0a,
   0x02, 0x28, 0x8a, 0x00, 0x91, 0x3e, 0xee, 0xfb, 0x3e, 0xe8, 0xfb, 0x00,
   0x91, 0x02, 0x08, 0x82, 0x22, 0x28, 0x82, 0x00, 0x9f, 0xbe, 0x0f, 0xfa,
   0x3e, 0xe8, 0x83, 0x00 };

void screen_command(uint8_t c){
	SCREEN_CS_LOW;
	SCREEN_INST_REG;
	for(uint8_t i=0; i<8; i++){
		SCREEN_CLOCK_LOW;
		if(c&128){
			SCREEN_DATA_HIGH;
		}
		else SCREEN_DATA_LOW;
		SCREEN_CLOCK_HIGH;
		c <<= 1;
	}
	SCREEN_CS_HIGH;
}

void screen_data_out(uint8_t c){
	SCREEN_CS_LOW;
	SCREEN_DATA_REG;
	for(uint8_t i=0; i<8; i++){
		SCREEN_CLOCK_LOW;
		if(c&128){
			SCREEN_DATA_HIGH;
		}
		else SCREEN_DATA_LOW;
		SCREEN_CLOCK_HIGH;
		c <<= 1;
	}
	SCREEN_CS_HIGH;
}


void screen_setcolumn(uint8_t c){
	screen_command(0x10|((c>>4)&0x0F));
	screen_command(c&0x0F);
}

void screen_setpage(uint8_t page){
	screen_command(0b10110000|page);
}

void screen_refresh_rect(uint8_t tlx, uint8_t tly, uint8_t brx, uint8_t bry){
	uint8_t t;
	if(tlx>brx){
		t = tlx;
		tlx = brx;
		brx = t;
	}
	if(tly>bry){
		t = tly;
		tly = bry;
		bry = t;
	}
	tly/=8;
	bry=(bry+1)/8;
	for(;tly<=bry;tly++){
		screen_setpage(tly);
		for(uint8_t column=tlx;column<=brx;column++){
			screen_setcolumn(column);
			screen_data_out(screen_buffer[tly][column]);
		}
	}
}

void screen_refresh(void){
	for(uint8_t page=0;page<8;page++){
		screen_setpage(page);
		for(uint8_t column=0;column<128;column++){
			screen_setcolumn(column);
			screen_data_out(screen_buffer[page][column]);
		}
	}
}


inline void screen_setpixel(uint8_t x, uint8_t y, uint8_t p){
	if(p&1) SET_BIT(screen_buffer[y/8][x],(y%8));
	else CLR_BIT(screen_buffer[y/8][x],(y%8));
}

void screen_init(void) {

	SCREEN_RESET;
	screen_command(0x40);
	screen_command(0xA1);
	screen_command(0xC0); screen_command(0xA6);
	screen_command(0xA2);
	screen_command(0x2F);
	screen_command(0xF8); screen_command(0x00);
	screen_command(0x27); screen_command(0x81); screen_command(0x10);
	screen_command(0xAC); screen_command(0x00);
	screen_command(0xAF);
}

uint8_t screen_getpixel(uint8_t x, uint8_t y){
	return screen_buffer[y/8][x]&(1<<(y%8));
}

void screen_draw_rect(uint8_t tlx, uint8_t tly, uint8_t brx, uint8_t bry,uint8_t p){
	uint8_t t;
	if(tlx>brx){
		t = tlx;
		tlx = brx;
		brx = t;
	}
	if(tly>bry){
		t = tly;
		tly = bry;
		bry = t;
	}
	for(uint8_t x=tlx;x<=brx;x++){
		for(uint8_t y=tly;y<=bry;y++){
			if(p==2){
				if(screen_getpixel(x,y)) screen_setpixel(x,y,0);
				else screen_setpixel(x,y,1);
			}
			else screen_setpixel(x,y,p);
		}
	}
}

void screen_blit_xbm(const uint8_t* image, const uint8_t iw, const uint8_t ih, uint8_t dx, uint8_t dy, uint8_t sx, uint8_t sy, const uint8_t w, const uint8_t h){
	for(uint8_t x=0; x<w; x++){
		for(uint8_t y=0; y<h; y++){
			screen_setpixel(dx+x, dy+y, xbm_getpixel(image, iw, ih, sx+x, sy+y));
		}
	}
}

void screen_draw_numeral(uint8_t* x, uint8_t y, uint8_t n){
	if(n>1) screen_blit_xbm(numerals_bits,57,5,*x,y,n*6-3,0,6,5);
	else if(n==0) screen_blit_xbm(numerals_bits,57,5,*x,y,0,0,6,5);
	else screen_blit_xbm(numerals_bits,57,5,*x,y,6,0,3,5);
	if(n==1) *x+=3;
	else *x+=6;
}

void screen_draw_number(uint8_t x, uint8_t y, uint16_t n){
	if(n>=10000) screen_draw_numeral(&x,y,n/10000);
	if(n>=1000) screen_draw_numeral(&x,y,(n%10000)/1000);
	if(n>=100) screen_draw_numeral(&x,y,(n%1000)/100);
	if(n>=10) screen_draw_numeral(&x,y,(n%100)/10);
	screen_draw_numeral(&x,y,n%10);
}

void screen_draw_temp(uint8_t x, uint8_t y, int16_t temp){
	if(temp<0) screen_draw_rect(x-4,y+2,x-2,y+2,1);
	temp=ABS(temp);
	uint8_t integral = temp/16;
	uint8_t fractional = round_fractionals[ABS(temp)%16];
	screen_draw_numeral(&x,y,integral/10);
	screen_draw_numeral(&x,y,integral%10);
	screen_setpixel(x,y+4,1);
	x+=2;
	screen_draw_numeral(&x,y,fractional/10);
	screen_draw_numeral(&x,y,fractional%10);
}

inline void screen_draw_letter(uint8_t x, uint8_t y, uint8_t c){
	screen_blit_xbm(letters_bits,155,5,x,y,(c-0x41)*6,0,6,5);
}