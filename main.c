#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#define F_CPU 1000000
#include <util/delay.h>
#include "oled.h"

#define ADC_IN      PB2
uint8_t sc = 0; 
ISR(TIM1_COMPA_vect)
{
  sc++;
}

void adc_init(void)
{
  //set Vref = Vcc
  ADMUX &= ~((1 << REFS2)|(1 << REFS1)|(1 << REFS0));
  //adjust result left
  //ADMUX |= (1 << ADLAR);
  //set ADC1 on PB2 as only input
  ADMUX |= (1 << MUX0);
  //set prescaler to 8, at 1MHz this is 125kHz well within the recommended range of 50-200kHz
  ADCSRA |= (1 << ADPS1)|(1 << ADPS0);
  //set signal source to free running mode
  ADCSRA |= (1 << ADATE);
  //enable adc
  ADCSRA |= (1 << ADEN);
  //set free running mode
  //ADCSRB &= ~((1 << ADTS0)|(1 << ADTS1)|(1 << ADTS2));
  //disable digital buffer on adc1 to reduce power consumption
  DIDR0 = (1 << ADC1D);
  //start conversion
  ADCSRA |= (1 << ADSC);
}

void timer_init(void)
{
  //reset TCnt1 to 0 after compare match and set prescaler to 2^12
  TCCR1 |= (1 << CTC1)|(1 << CS13)|(1 << CS12)|(1 << CS10);
  //sets compare register A so interrupt will occur as close to 1/sec as possible 
  OCR1A = 244;
  //enable timer 1 register A compare match interrupt
  TIMSK = (1 << OCIE1A);
  //interrupt frequency will be 1.00057633 Hz
  //to keep accurate time, subtract 25 seconds every 12 hours and add 13 seconds every 56 days
}
//make no assumptions about ADMUX and ADCSRA and leave them unchanged
uint16_t read_vcc(void)
{
  //store old admux settings
  uint8_t admux_old = ADMUX;
  //store old adcsra settings
  uint8_t adcsra_old = ADCSRA;
  //store old adcsrb settings
  uint8_t adcsrb_old = ADCSRB;
  //select ADC4 for temp measurement
  ADMUX = 0x0F; 
  //set vref to internal 1.1V reference
  ADMUX |= (1 << REFS1);
  _delay_ms(100);
  ADCSRA = 0x00;
  //set prescaler to 8
  ADCSRA |= (1 << ADPS1)|(1 << ADPS0);
  //set to free running mode
  ADCSRB &= ~((1 << ADTS0)|(1 << ADTS1)|(1 << ADTS2));
  //set signal source
  ADCSRA |= (1 << ADATE);
  //enable adc
  ADCSRA |= (1 << ADEN);
  ADCSRA |= (1 << ADSC);
  while (!(ADCSRA & (1 << ADSC)));
  uint16_t measured_11 = ADC;
  ADCSRA &= ~(1 << ADEN);
  ADCSRA |= (1 << ADEN);
  //set vref to vcc 
  ADMUX &= ~(1 << REFS1);
  _delay_ms(100);
  ADCSRA |= (1 << ADSC);
  while (!(ADCSRA & (1 << ADSC)));
  uint16_t measured_vcc = ADC;
  uint16_t vccx10 = ((measured_11 *100) / measured_vcc)*11; 
  //restore old admux settings
  ADMUX = admux_old;
  //restore old adcsra settings
  ADCSRA = adcsra_old;
  //restore old adcsrb settings
  ADCSRB = adcsrb_old;
  return vccx10;
}

int main(void)
{
  cli();
  adc_init();
  timer_init();
  oled_init();
  uint16_t measured;
  sei();
  oled_fillscreen(0x00);
  //somewhat useless function rn
  oled_setpos(0,0);
  oled_string_8x16(0,0,"Vcc is:");
  //oled_num_8x16(8,4,read_vcc());
  while(1)
  {
    if ((ADCSRA & (1 << ADSC))) {measured = ADC;}
    oled_num_8x16(8,4,read_vcc());
    _delay_ms(10);
  }
}
