#include <avr/io.h>
#include <avr/interrupt.h>

#define adc_3v ((1<<MUX2)|(1<<MUX1))
#define adc_5v ((1<<MUX1)|(1<<MUX0))
#define adc_12v (1<<MUX1)  
#define adc_degur_5v ((1<<MUX2)|(1<<MUX1)|(1<<MUX0))
#define adc_pwm_lvl (1<<MUX2)
#define adc_mux_clear 0b11110000

#define _3v 			0
#define _5v 			1
#define _12v 			2
#define _degur_5v 		3
#define _pwm_v			4
#define adc_max 		5


#define turn_ON_PS		0

#define PS_ON 0
#define PWR_OK 0

unsigned char voltage_array[5];
unsigned char adc_index=0;
unsigned char led_index=0;
unsigned char value_index=0;
unsigned char LED[3]={4,8,9};
unsigned char number_B[10]={
							0b11000000,//0
							0b11100101,//1
							0b11001000,//2
							0b11000001,//3
							0b11100101,//4
							0b11010001,//5
							0b11010000,//6
							0b11000101,//7
							0b11000000,//8
							0b11000001//9
							};
unsigned char number_D[10]={
							0b01111111,//0
							0b11111111,//1
							0b10111111,//2
							0b10111111,//3
							0b00111111,//4
							0b00111111,//5
							0b00111111,//6
							0b11111111,//7
							0b00111111,//8
							0b00111111//9
							};

unsigned char DP_D =0b11011111;


void init_GPIO()
{

	DDRC = 0b11100011;
	PORTC =0b11100011;

	DDRB = 0b11111111;
	PORTB = 0b00000000;

	DDRD = 0b11110111;
	PORTD = 0b11110000;

}
void change_adc_input(unsigned char input)
{
	switch (input)
	{
		case _3v : {
					ADMUX =  ADMUX & adc_mux_clear;//clear adc MUX
					ADMUX |= adc_3v;
					break;
					}
		case _5v : {
					ADMUX =  ADMUX & adc_mux_clear;//clear adc MUX
					ADMUX |= adc_5v;
					break;
					}
		case _12v : {
					ADMUX =  ADMUX & adc_mux_clear;//clear adc MUX
					ADMUX |= adc_12v;
					break;
					}
		case _degur_5v : {
							ADMUX =  ADMUX & adc_mux_clear;//clear adc MUX
							ADMUX |= adc_degur_5v;
							break;
							}
		case _pwm_v : {
						ADMUX =  ADMUX & adc_mux_clear;//clear adc MUX
						ADMUX |= adc_pwm_lvl;
						break;
						}
	}
}
void show_led_value()
{
	unsigned char tmp=0x00;
	tmp = PORTB;
	tmp= tmp&0b00000010;
	tmp = tmp|number_B[LED[led_index]];
//PORT B
	for (int i=0;i<8;i++)
	{
		if((tmp&0b00000001)==0x01)
		{
			PORTB |=(1<<i);
		}
		else
			{
				PORTB &=~(1<<i);
			}
			tmp = (tmp>>1);

	}
//port D
	if (led_index==0)
	{
		tmp = number_D[LED[led_index]];
		tmp=tmp&DP_D;
	}else
		{
			tmp = number_D[LED[led_index]];
		}
//port D
	tmp = (tmp>>6);
	for (int i=6;i<8;i++)
	{
		if((tmp&0b00000001)==0x01)
		{
			PORTD |=(1<<i);
		}
		else
			{
				PORTD &=~(1<<i);
			}
			tmp = (tmp>>1);

	}


	switch 	(led_index)
	{
		case 0:{
				
				PORTC &=~((1<<0)|(1<<1)|(1<<5));
				PORTC |=(1<<0);
				break;
				}
		case 1:{
				PORTC &=~((1<<0)|(1<<1)|(1<<5));
				PORTC |=(1<<1);
				break;
				}
		case 2:{
				PORTC &=~((1<<0)|(1<<1)|(1<<5));
				PORTC |=(1<<5);
				break;
				}
	}
	led_index++;
	if(led_index>=3){led_index=0;}


}
void init_timer_0()
{
	TCNT0=0x00;
//	TCCR0|=((1<<CS02)|(1<<CS01)|(1<<CS00)); //set prescaller 1024
	TIMSK|=(1<<TOIE0);
	TCCR0|=(1<<CS00)|(1<<CS01);
//TCCR0|=(1<<CS01);
}

void init_measuring_timer_1()
{
	TCNT1=0x00;
}


void init_adc()
{
	ADMUX|=((1<<REFS1)|(1<<REFS0));// internal 2.56
	//ADMUX|=(1<<REFS0);//  external AVCC
	ADMUX|=(1<<ADLAR);// shift left

	ADCSRA|=((1<<ADIE)|(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)); // ENABLE ADC, ENABLE ADC INTERRUPT, SET PRESCALER 128
	
	ADMUX =  ADMUX & adc_mux_clear;//clear adc MUX
}

ISR (ADC_vect)
{
	cli();
	//adc_data[adc_counter]=ADCH;
	//adc_counter++;
//	ADCSRA|=(1<<ADSC);
	sei();
	//uart_transmit(ADCH);
	voltage_array[0]=1;
}

ISR(INT1_vect)
{
cli();
	unsigned int temp;
	temp = ((voltage_array[value_index]*256)/256);
	LED[0] = temp /100;
	temp = temp %100;
	LED[1] = temp/10;
	LED[2] = temp%10;

		value_index++;	
	if(value_index>5){value_index=0;}
sei();
}

ISR(INT0_vect)
{
}

ISR (TIMER0_OVF_vect)
{
	//PORTB= 0xff;
	cli();
	change_adc_input(adc_index);
	if(adc_index<adc_max)
	{
		adc_index++;
	}else
		{
			adc_index=0;
		}
	show_led_value();
	sei();
	
}
void init_interrupt()
{
	GICR|=((1<<INT1)|(1<<INT0));
	MCUCR|=((1<<ISC11)|(1<<ISC01));
}

int main ()
{
	init_GPIO();
	 init_interrupt();
	 init_adc();
	 init_timer_0();
	sei();
	
	PORTD |= (1<<turn_ON_PS);
	
	while(1)
	{
		//PORTC=PORTC;
		
	}
}
