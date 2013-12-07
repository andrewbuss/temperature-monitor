#pragma once 

#define CLR_BIT(p,n) ((p) &= ~((1) << (n)))
#define HALT printf("HALT\n"); while(1){}
#define SET_BIT(p,n) ((p) |=  ((1) << (n)))

#ifndef outb
	#define	outb(addr, data)	addr = (data)
#endif
#ifndef inb
	#define	inb(addr)			(addr)
#endif
#ifndef outw
	#define	outw(addr, data)	addr = (data)
#endif
#ifndef inw
	#define	inw(addr)			(addr)
#endif
#ifndef BV
	#define BV(bit)			(1<<(bit))
#endif

#ifndef cli
	#define cli()			__asm__ __volatile__ ("cli" ::)
#endif
#ifndef sei
	#define sei()			__asm__ __volatile__ ("sei" ::)
#endif

#define GNUC_PACKED __attribute__((packed)) 
#define DDR(x) ((x)-1)
#define PIN(x) ((x)-2)
#define MIN(a,b)			((a<b)?(a):(b))
#define MAX(a,b)			((a>b)?(a):(b))
#define ABS(x)				((x>0)?(x):(-x))

void delay_us(uint16_t x){
	for(;x>0;x--){
		for(uint8_t y=0;y<10;y++){asm volatile("nop");}
	}
}

void delay_ms(uint16_t x){
	for (; x > 0 ; x--){
		delay_us(250);
		delay_us(250);
		delay_us(250);
		delay_us(250);
	}
}