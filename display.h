#include "screen.h"
#include <avr/interrupt.h>
#include "eeprom.h"
#include "data.h"
#include "common.h"
#include <math.h>
#include "tmp102.h"

uint8_t graph_yzero = 40;
uint8_t graph_xzero = 0;
uint8_t therm_xzero = 50;
int16_t graph_time_back = 0;
uint8_t display_range = 0;
uint8_t display_mode = 7;

void display_draw_graph(void){
	uint8_t x = 127;
	if(display_range == 0){
		while(x>16){
			x--;
			if(x%2 == 1) screen_setpixel(x,graph_yzero,1);
			if(x%12 == 7) screen_draw_rect(x,graph_yzero-1,x,graph_yzero+1,1);
			if(127-x==12*7){
				graph_xzero = x;
				break;
			}
		}
	}else if(display_range == 1){
		while(x>16){
			x--;
			if(x%2 == 1) screen_setpixel(x,graph_yzero,1);
			if(x%4 == 3) screen_draw_rect(x,graph_yzero-1,x,graph_yzero+1,1);
			if(127-x==24*4){
				graph_xzero = x;
				break;
			}
		}		
	}else if(display_range == 2){
		while(x>16){
			x--;
			if(x%5 == 2) screen_setpixel(x,graph_yzero,1);
			if(x%15 == 7) screen_draw_rect(x,graph_yzero-1,x,graph_yzero+1,1);
			if(127-x==15*6){
				graph_xzero = x;
				break;
			}
		}
	}else if(display_range == 3){
		while(x>16){
			x--;
			if(x%2 == 1) screen_setpixel(x,graph_yzero,1);
			if(x%10 == 7) screen_draw_rect(x,graph_yzero-1,x,graph_yzero+1,1);
			if(127-x==10*6){
				graph_xzero = x;
				break;
			}
		}
	}
	int8_t y = -63+graph_yzero;
	while(graph_yzero-y>0){
		y++;
		if(y%2==0) screen_setpixel(graph_xzero,graph_yzero-y,1);
		if(y%10==0 && y) screen_draw_rect(graph_xzero,graph_yzero-y,graph_xzero+1,graph_yzero-y,1);
	}	
}

void display_draw_data(uint8_t sensor){
	uint8_t x = 127;
	uint8_t data_zoom = 1;
	uint16_t address = (pn<<8)|pi;
	if(display_range == 0) data_zoom = 120;
	else{
		if(display_range == 2) data_zoom = 4;
		else if(display_range == 1) data_zoom =  15;
		data_skip_back(&address,-graph_time_back);
	}	
	while(1){
		if(x<=graph_xzero) break;
		uint16_t raw = data_get_live(address+sensor);
		int8_t temp = (int8_t)(((float)raw)/16.0);
		if(raw!=0x0FFF && graph_yzero-temp<63 && graph_yzero-temp>0){
			uint8_t y = graph_yzero-temp;
			screen_setpixel(x,y,1);
		}
		x--;
		data_skip_back(&address,data_zoom);
	}
}
void display_draw_time_segment(uint8_t* x, uint8_t y, uint8_t n){
	screen_draw_numeral(x,y,(n%100)/10);
	screen_draw_numeral(x,y,n%10);
}

void display_draw_time(uint8_t x, uint8_t y){
	
}

void display_refresh(void){
	screen_init();
	screen_draw_rect(0,0,127,63,0);
	if(display_mode){
		for(uint8_t i=0;i<6;i++) screen_draw_rect(3+i,55-i,3+i,55+i,1);
		screen_draw_rect(8,53,14,57,1);
		for(uint8_t i=0;i<6;i++) screen_draw_rect(14-i,40-i,14-i,40+i,1);
		screen_draw_rect(3,38,8,42,1);
		display_draw_graph();
		if(display_mode&4){
			screen_draw_letter(6,3,'A');
			display_draw_data(0);
		}
		if(display_mode&2){
			screen_draw_letter(3,9,'B');
			display_draw_data(2);
		}
		if(display_mode&1){
			screen_draw_letter(9,9,'C');
			display_draw_data(4);
		}
		if(display_range==0){
			screen_draw_number(6,18,7);
			screen_draw_letter(6,24,'D');
		}
		else if(display_range==1){
			screen_draw_number(3,18,24);
			screen_draw_letter(3,24,'H');
			screen_draw_letter(9,24,'R');
		}
		else if(display_range==2){
			screen_draw_number(6,18,6);
			screen_draw_letter(3,24,'H');
			screen_draw_letter(9,24,'R');
		}
		else if(display_range==3){
			screen_draw_number(7,18,1);
			screen_draw_letter(3,24,'H');
			screen_draw_letter(9,24,'R');
		}
		screen_draw_rect(graph_xzero+1,0,126,0,1);
		screen_draw_rect(graph_xzero+1,63,126,63,1);
		screen_draw_rect(127,0,127,63,1);
		if(display_range!=0){
			uint8_t x = 70;
			uint8_t y = 2;
			screen_draw_numeral(&x,y,(-graph_time_back)/1440);
			screen_draw_letter(x,y,'D');
			x+=8;
			display_draw_time_segment(&x,y,((-graph_time_back)%1440)/60);
			screen_setpixel(x,y+1,1);
			screen_setpixel(x,y+3,1);
			x+=2;
			display_draw_time_segment(&x,y,((-graph_time_back)%60));
		}
		screen_draw_number(graph_xzero-12,graph_yzero-32, 30);
		screen_draw_number(graph_xzero-12,graph_yzero-22, 20);
		screen_draw_number(graph_xzero-9,graph_yzero-12, 10);
		screen_draw_number(graph_xzero-6,graph_yzero-2, 0);
		screen_draw_number(graph_xzero-9,graph_yzero+8, 10);
		screen_draw_number(graph_xzero-12,graph_yzero+18, 20);
		screen_draw_rect(graph_xzero-15,graph_yzero+10,graph_xzero-13,graph_yzero+10,1);
		screen_draw_rect(graph_xzero-18,graph_yzero+20,graph_xzero-16,graph_yzero+20,1);
	}else{
		SET_BIT(PORTD,7);
		screen_draw_letter(3,3,'L');
		screen_draw_letter(9,3,'I');
		screen_draw_letter(3,9,'V');
		screen_draw_letter(9,9,'E');
		screen_draw_letter(20,8,'A');
		screen_draw_letter(20,28,'B');
		screen_draw_letter(20,48,'C');
		screen_draw_rect(therm_xzero,8,therm_xzero,12,1);
		screen_draw_rect(therm_xzero,28,therm_xzero,32,1);
		screen_draw_rect(therm_xzero,48,therm_xzero,52,1);
		for(uint8_t x=30;x<=127;x++){
			if(ABS(therm_xzero-x)%10==0){
				screen_draw_rect(x,9,x,11,1);
				screen_draw_rect(x,29,x,31,1);
				screen_draw_rect(x,49,x,51,1);
			}else if(ABS(therm_xzero-x)%2==0){
				screen_setpixel(x,10,1);
				screen_setpixel(x,30,1);
				screen_setpixel(x,50,1);
			}
		}
		printf_P(PSTR("\npulling data from sensors...\n"));
		int16_t raw = 0x0FFF;
		r1:
		raw = tmp102_readraw(0x90);
		if(raw == 0x0FFF) goto r1;
		int8_t temp = (int8_t)(((float)raw)/16.0);
		if(temp/16 < 70){
			screen_draw_rect(therm_xzero+temp-1,7,therm_xzero+temp+1,13,2);
			screen_draw_temp(therm_xzero+temp-12,15,raw);
		}
		r2: 
		raw = tmp102_readraw(0x92);
		if(raw == 0x0FFF) goto r2;
		temp = (int8_t)(((float)raw)/16.0);
		if(temp/16 < 70){
			screen_draw_rect(therm_xzero+temp-1,27,therm_xzero+temp+1,33,2);
			screen_draw_temp(therm_xzero+temp-12,35,raw);
		}
		r3:
		raw = tmp102_readraw(0x94);
		if(raw == 0x0FFF) goto r3;
		temp = (int8_t)(((float)raw)/16.0);
		if(temp/16 < 70){
			screen_draw_temp(therm_xzero+temp-12,55,raw);
			screen_draw_rect(therm_xzero+temp-1,47,therm_xzero+temp+1,53,2);
		}
		screen_draw_rect(therm_xzero-20-9,3,therm_xzero-20-7,3,1);
		screen_draw_number(therm_xzero-20-5,1,20);
		screen_draw_number(therm_xzero-2,1,0);
		screen_draw_number(therm_xzero+20-5,1,20);
		screen_draw_number(therm_xzero+40-5,1,40);
		screen_draw_number(therm_xzero+60-5,1,60);
		CLR_BIT(PORTD,7);
	}
	screen_refresh();
}