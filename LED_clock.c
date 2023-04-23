#include<8051.h>
#include <at89s8252.h>

#define TRUE 1
#define FALSE 0
#define T0_DAT 65536-921
#define TL_0 T0_DAT%256
#define TH_0 T0_DAT/256
#define T100 100         // półokres LED

unsigned char sekundy;
unsigned char minuty;
unsigned char godziny;
unsigned char timer_buf;
unsigned int licznik_przerwan;
unsigned char Keyboard[4] = {0,0,0,0};

__sfr __at(0x87) PCON;
__sfr __at(0x98) SCON;
__sfr __at(0x89) TMOD;
__sfr __at(0x8C) TH0;
__sfr __at(0x8A) TL0;
__sfr __at(0x8D) TH1;
__sfr __at(0x8B) TL1;

__sbit __at(0xA9) ET0;
__sbit __at(0xAC) ES;
__sbit __at(0xAF) EA;
__sbit __at(0x8C) TR0;
__sbit __at(0x8E) TR1;
__sbit __at(0x96) SEG_OFF;
__sbit __at(0x97) LED;
__sbit __at(0x98) RI;   //flaga koniec odb. UART
__sbit __at(0x99) TI;   //flaga koniec nad. UART
__sbit __at(0xB5) MUXK;

__bit rec_flag;
__bit send_flag;
__bit onesecond_flag;
__bit t0_flag;

__code unsigned char WZOR[10]= {0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110, 0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111};
unsigned char ARRAY[6];

void clock_values(void);
void updateTime(void);
void obslugaKlaw(void);

void clock_values(){
    ARRAY[0] = sekundy%10;
    ARRAY[1] = (unsigned char)(sekundy / 10);
    ARRAY[2] = minuty%10;
    ARRAY[3] = (unsigned char)(minuty / 10);
    ARRAY[4] = godziny%10;
    ARRAY[5] = (unsigned char)(godziny / 10);
}
void updateTime(){
	if (onesecond_flag == TRUE) {
		onesecond_flag = FALSE;
		licznik_przerwan -= 900;
		sekundy++;
		if (sekundy == 60) {
			minuty++;
			sekundy = 0;
			if (minuty == 60) {
  			    godziny++;
				minuty = 0;
				if (godziny == 24)
					godziny = 0;
			}
		}
	}
}
void t0_int()__interrupt(1) {
    TH0 = 252;
	t0_flag = TRUE;
	licznik_przerwan++;
	if (licznik_przerwan >= 900) {
		onesecond_flag = TRUE;
	}
}
void obslugaKlaw(void){
        if(Keyboard[0] == (0b100001)){
            if(godziny < 23){
                godziny++;
            }
			else{
                godziny = 0;
            }
            clock_values();
        }
        else if(Keyboard[0] == (0b100010)){
		    if(godziny > 0){
	            godziny--;
            }
			else{
                godziny = 23;
            }
            clock_values();
        }
        if(Keyboard[0] == (0b10001)){
            if (minuty < 59){
                 minuty++;
            }
			else{
                 minuty = 0;
            }
            clock_values();
        }
        else if(Keyboard[0] == (0b10010)){
            if(minuty > 0){
                minuty--;
            }
			else{
                minuty = 59;
            }
            clock_values();
        }
        if(Keyboard[0] == (0b101)){
            if(sekundy < 59){
                sekundy++;
            }
			else{
                sekundy = 0;
            }
            clock_values();
        }
        else if(Keyboard[0] == (0b110)){
            if(sekundy > 0){
                sekundy--;
            }
			else{
                sekundy = 59;
            }
            clock_values();
            updateTime();
        }
}

void main() {
  __xdata unsigned char *led_wyb = (__xdata unsigned char *) 0xFF30;
  __xdata unsigned char *led_led = (__xdata unsigned char *) 0xFF38;
    
  unsigned char led_b;
  unsigned char led_p;
  
  licznik_przerwan = 0;
	sekundy = 52;
	minuty = 59;
	godziny = 13;

	PCON = 0x80;
	SCON = 0b01010000;
	TMOD = 0b00100001;
	TL0 = 0;
	TH0 = 252;
	TL1 = 0xFD;
	TH1 = 0xFD;

	send_flag = FALSE;
	rec_flag = FALSE;
	t0_flag = FALSE;
	ET0 = TRUE;
	ES = TRUE;
	EA = TRUE;
	TR0 = TRUE;
	TR1 = TRUE;
	LED = TRUE;
  clock_values();
  updateTime();

    while(TRUE	) {
            if(t0_flag) {
            	t0_flag = FALSE;
            	for (led_p = 0, led_b = 1; led_p < 6; led_p++) {
					SEG_OFF = TRUE;
					*led_wyb = led_b;
					*led_led = WZOR[ARRAY[led_p]];
					SEG_OFF = FALSE;

					if(P3_5) Keyboard[0]=(Keyboard[0] | led_b);
					led_b+=led_b;
				}
                if(led_b == 64){
            		   led_b=1;
              		   led_p=0;
           			   if(Keyboard[0]==Keyboard[1] && Keyboard[0]==Keyboard[2] && Keyboard[0]!=Keyboard[3] && Keyboard[0]!=0){
                          obslugaKlaw();
                       }
                       Keyboard[3]=Keyboard[2];
				   	   Keyboard[2]=Keyboard[1];
    				   Keyboard[1]=Keyboard[0];
    				   Keyboard[0]=0;
     	        }
            clock_values();
            updateTime();
            }
        }
}
