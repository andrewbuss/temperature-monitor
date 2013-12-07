#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "screen.h"
#include "display.h"
#include "eeprom.h"
#include "tmp102.h"
#include "uart_stdio.h"
#include "data.h"

#define SERIAL_INTERFACE

#define do_nothing() 
#ifndef SERIAL_INTERFACE
	#define printf_P(...) do_nothing()
#endif

uint16_t uptime_minutes = 0;
int uptime_seconds = 0;
uint16_t uptime_days = 0;

#define RETRY_SENSOR1 0
#define RETRY_SENSOR2 1
#define RETRY_SENSOR3 1

uint8_t log_automatically = 1;
uint8_t time_multiplier = 1;
uint8_t pinc_previous = 0xff;

void reconfigure_tmp102s(void){
	while(tmp102_configure(0x90)==0xFFF){}
    while(tmp102_configure(0x90)==0xFFF){}
 	while(tmp102_configure(0x92)==0xFFF){}
	while(tmp102_configure(0x92)==0xFFF){}
	while(tmp102_configure(0x94)==0xFFF){}
	while(tmp102_configure(0x94)==0xFFF){}
	while(tmp102_configure(0x96)==0xFFF){}
    while(tmp102_configure(0x96)==0xFFF){}
}

int main(void){
	DDRB = 0b11101110;
    DDRC = 0b11110000;
    DDRD = 0b11111110; 
	PORTC = 0b00001111;	
	PORTD = 0b01000000;
	PORTB = 0b00000001;
	OCR1A = 0x8000;
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    TCCR1B |= (1 << CS12);
    #ifdef SERIAL_INTERFACE
    uart_init(UART_BAUD_SELECT(38400,F_CPU));
    uart_stdio_init();
    #endif
    sei();
    SET_BIT(PORTD,7);
    printf_P(PSTR("\nATMega started\nset up UART\n"));
    printf_P(PSTR("resetting EEPROM\n"));
    eeprom_reset();
    printf_P(PSTR("initializing i2c\n"));
    i2cInit();
    printf_P(PSTR("configuring temperature sensors at 0x90, 0x92, 0x94, 0x96\n"));
    reconfigure_tmp102s();
    printf_P(PSTR("seeking spot in EEPROM\n"));
	while(1){
		uint16_t address = (pn<<8) | pi;
		if(eeprom_readbyte(address)==0x0F && eeprom_readbyte(address+1)==0xFF &&
			eeprom_readbyte(address+2)==0x0F && eeprom_readbyte(address+3)==0xFF &&
			eeprom_readbyte(address+4)==0x0F && eeprom_readbyte(address+5)==0xFF) break;
		if(pi<234) pi+=6;
		else{
			pi = 0;
			pn++;
			if(pn==0){
				printf_P(PSTR("couldn't find place to start!\n")); break;
			}
		}
	}
	printf_P(PSTR("starting at 0x%02X%02X\n"), pn,pi);
	data_load_page(pn);
	display_refresh();
	CLR_BIT(PORTD,7);
	while(1){
		#ifdef SERIAL_INTERFACE
		uint16_t c;
		c = uart_getc();
		if(!(c & UART_NO_DATA)){
			uint8_t key = c&255;
			if(key=='z'){
				printf_P(PSTR("print cache\n"));
				uint8_t i=0;
				while(1){
					printf_P(PSTR("%02X "),cache[i]);
					i++;
					if(i%6 == 0) printf_P(PSTR("\n"));
					if(i==0) break;
				}
			}else if(key=='y'){
			 	printf_P(PSTR("Set time multiplier: "));
			 	scanf("%d",&time_multiplier);
			 	printf_P(PSTR("%d\n"),time_multiplier);
			}else if(key=='t'){
				int address = 0;
				printf_P(PSTR("read sensor address: 0x"));
				scanf("%02X",&address);
				printf_P(PSTR("%02X\n"),address);
				int16_t raw = 0x0FFF;
				while(raw == 0x0FFF) tmp102_readraw(address);
				float temp = raw/16.0;
				printf_P(PSTR("sensor 0x%02X is: %04X, %.2f C, %.1f F\n"),address,raw,temp,temp*1.8+32);
			}else if(key=='n'){
				printf_P(PSTR("reset page: 0x"));
				scanf("%02X",&pn);
				printf_P(PSTR("%02X\n"),pn);
				data_reset_cache();
				data_flush_cache();
			}else if(key=='x'){
				printf_P(PSTR("resetting cache\n"));
				data_reset_cache();
			}else if(key=='m'){
				printf_P(PSTR("print page: 0x"));
				int tpn;
				scanf("%02X",&tpn);
				printf_P(PSTR("%02X\n"),tpn);
				printf_P(PSTR("\n"));
				uint8_t i=0;
				while(1){
					uint16_t address = (tpn<<8)|i;
					printf_P(PSTR("%02X "),eeprom_readbyte(address));
					i++;
					if(i%16 == 0) printf_P(PSTR("\n"));
					if(i==0) break;
				}
			}else if(key=='c'){
				printf_P(PSTR("%04dd %02d:%02d:%02d since startup\n"),uptime_days, uptime_minutes/60, uptime_minutes%60, uptime_seconds);
			}else if(key=='q'){
				printf_P(PSTR("logging automatically\n"));
				log_automatically = 1;
			}else if(key=='a'){
				printf_P(PSTR("not logging automatically\n"));
				log_automatically = 0;
			}else{
				printf_P(PSTR("\n"));
			}
		}
		#endif
		uint8_t pinc_changed = (PINC^pinc_previous)&15;
		if(pinc_changed == 0) continue;
		pinc_previous = PINC;
		if(pinc_changed & 1) screen_draw_rect(1,1,15,15,2);
		if(pinc_changed & 2) screen_draw_rect(1,16,15,30,2);
		if(pinc_changed & 4) screen_draw_rect(1,33,15,47,2);
		if(pinc_changed & 8) screen_draw_rect(1,48,15,62,2);
		if(pinc_changed & PINC & 1){
	 		if(display_mode == 0) display_mode = 4;
	 		else if(display_mode == 4) display_mode = 2;
	 		else if(display_mode == 2) display_mode = 1;
	 		else if(display_mode == 1) display_mode = 6;
	 		else if(display_mode == 6) display_mode = 3;
	 		else if(display_mode == 3) display_mode = 5;
	 		else if(display_mode == 5) display_mode = 7;
	 		else if(display_mode == 7) display_mode = 0;
	 		display_refresh();
	 	}
	 	else if(pinc_changed & PINC & 2  && display_mode > 0){
	 		display_range++;
	 		if(display_range == 4) display_range = 0;
	 		display_refresh();
	 	}
	 	else if(pinc_changed & PINC & 4  && display_mode > 0){
	 		if(display_range == 1) graph_time_back+= 360;
	 		else if(display_range == 2) graph_time_back+=90;
	 		else if(display_range == 3) graph_time_back+=15;
	 		if(display_range == 1 && graph_time_back < -10080+1440) graph_time_back = -10080+1440;
		 	else if(display_range == 2 && graph_time_back < -10080+360) graph_time_back = -10080+360;
		 	else if(display_range == 3 && graph_time_back < -10080+60) graph_time_back = -10080+15;
		 	if(graph_time_back>0) graph_time_back = 0;
		 	display_refresh();
	 	}
	 	else if(pinc_changed & PINC& 8  && display_mode > 0){
	 		if(display_range == 1) graph_time_back-= 360;
	 		else if(display_range == 2) graph_time_back-=90;
	 		else if(display_range == 3) graph_time_back-=15;
	 		if(display_range == 1 && graph_time_back < -10080+1440) graph_time_back = -10080+1440;
	 		else if(display_range == 2 && graph_time_back < -10080+360) graph_time_back = -10080+360;
		 	else if(display_range == 3 && graph_time_back < -10080+60) graph_time_back = -10080+15;
		 	if(graph_time_back>0) graph_time_back = 0;
	 		display_refresh();
	 	}else{
	 		screen_refresh_rect(1,1,16,62);
	 	}
	 	pinc_previous = PINC;		
	}

}
ISR(TIMER1_COMPA_vect){
	uptime_seconds += time_multiplier;
	if(uptime_seconds >= 60){
		uptime_seconds = 0;
		uptime_minutes++;
		if(uptime_minutes==1440){
			uptime_minutes = 0;
			uptime_days++;
		}
		if(log_automatically){
			sei();
			SET_BIT(PORTD,7);
			printf_P(PSTR("\npulling data from sensors...\n"));
			int16_t a = 0x0FFF,b=0x0FFF,c=0x0FFF;
			r1:
			a = tmp102_readraw(0x90);
			if(a == 0x0FFF && RETRY_SENSOR1) goto r1;
			r2: 
			b = tmp102_readraw(0x92);
			if(b == 0x0FFF && RETRY_SENSOR2) goto r2;
			r3:
			c = tmp102_readraw(0x94);
			if(c == 0x0FFF && RETRY_SENSOR3) goto r3;
			printf_P(PSTR("sensor 0x90: %04X\n"),a);
			printf_P(PSTR("sensor 0x92: %04X\n"),b);
			printf_P(PSTR("sensor 0x94: %04X\n"),c);
			data_log(a,b,c);
			display_refresh();
			CLR_BIT(PORTD,7);
		}
	}else if(display_mode==0){
		display_refresh();
	}
}