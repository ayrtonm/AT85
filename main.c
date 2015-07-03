#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#define F_CPU 1000000
#include <util/delay.h>
#include "oled.h"

#define SW1 0x01
#define SW2 0x02
#define SW3 0x04

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
//makes no assumptions about ADMUX and leave it unchanged
uint16_t read_vcc(void)
{
  //store old admux settings
  uint8_t admux_old = ADMUX;
  //read Vbg (bandgap voltage) using Vcc as Vref
  ADMUX = 0x0C;
  _delay_ms(100);
  ADCSRA |= (1 << ADSC);
  while (!(ADCSRA & (1 << ADSC)));
  uint16_t measured = ADC;
  ADMUX = admux_old;
  uint16_t vccx10 = ((11*1023)/measured);
  return vccx10;
}

uint8_t read_buttons(uint16_t measured)
{
  uint8_t buttons = 0x00;
  if (measured > 549)
  {
    if (measured > 751)
    {
      if (measured > 920);//no buttons pressed, don't modify return value 
      else
      {
        buttons |= SW3;
      }
    }
    else
    {
      if (measured > 635)
      {
        buttons |= SW2;
      }
      else
      {
        buttons |= SW2|SW3;
      }
    }
  }
  else
  {
    if (measured > 432)
    {
      if (measured > 483)
      {
        buttons |= SW1;
      }
      else
      {
        buttons |= SW1|SW3;
      }
    }
    else
    {
      if (measured > 391)
      {
        buttons |= SW1|SW2;
      }
      else
      {
        buttons |= SW1|SW2|SW3; 
      }
    }
  }
  return buttons;
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
  oled_string_8x16(0,0,"this is scrollin");
  //oled_num_8x16(8,4,read_vcc());
  //set scroll conditions
  oled_send_command(0x26);
  oled_send_command(0x00);
  oled_send_command(0x00);
  oled_send_command(0b00000111);
  oled_send_command(0x01);
  oled_send_command(0x00);
  oled_send_command(0xff);
  //activate scroll
  oled_send_command(0x2f);
  //change contrast
  //apparently setting contrast to 0x00 does not make text black
  oled_send_command(0x81);
  oled_send_command(0x00);
  while(1)
  {
    if ((ADCSRA & (1 << ADSC))) {measured = ADC;}
    oled_string_8x16(0,4,"vcc:");
    oled_num_8x16(8*4,4,read_vcc());
    oled_num_8x16(16,2,read_buttons(measured));
    oled_string_8x16(0,6,"time:");
    oled_num_8x16(8*5,6,sc);
    //stop scrolling top line after 100 seconds
    if (sc == 100) 
    {
      oled_send_command(0x2e);
    }
    //inverts display every ten seconds
    if (!(sc % 10))
    {
      oled_send_command(0xa6+ ((sc / 10) % 2));
    }
    _delay_ms(10);
  }
}
