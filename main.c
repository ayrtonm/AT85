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
uint8_t mn = 0; 
uint8_t hr = 12; 
ISR(TIM1_COMPA_vect)
{
  if(sc++ == 59)
  {
    sc = 0;
    if (mn++ == 59)
    {
      mn = 0;
      if (hr++ == 12)
      {
        hr = 1;
      }
    }
  }
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
  uint8_t s = 0;
  cli();
  adc_init();
  timer_init();
  oled_init();
  uint16_t measured;
  sei();
  oled_fillscreen(0x00);
  //oled_num_8x16(8,4,read_vcc());
  //set scroll conditions
  oled_send_command(0x26);
  oled_send_command(0x00);
  oled_send_command(0x00);
  oled_send_command(0b00000111);
  oled_send_command(0x01);
  oled_send_command(0x00);
  oled_send_command(0xff);
  //deactivate scroll
  oled_send_command(0x2e);
  oled_send_command(0xa1);
  //change contrast
  //apparently setting contrast to 0x00 does not make text black
  oled_send_command(0x81);
  oled_send_command(0x00);
  for (;;)
  {
    _delay_ms(1);
    if ((ADCSRA & (1 << ADSC))) {measured = ADC;}
    if (s != read_buttons(measured))
    {
      s = read_buttons(measured);
      oled_fillscreen(0x00);
    }
    if (read_buttons(measured) & SW3)
    {
      oled_string_8x8(0,4,"vcc:");
      uint8_t v = read_vcc(); 
      oled_num_8x8(4,4,v/10);
      oled_string_8x8(5,4,".");
      oled_num_8x8(6,4,v%10);
      oled_string_8x8(7,4,"volts");
    }
    else if (read_buttons(measured) & SW2)
    {
      oled_string_8x8(0,0,"Title");
    }
    else if (!read_buttons(measured))
    {
      oled_num_8x8(0,6,hr);
      oled_string_8x8(2,6,":");
      oled_num_8x8(3,6,mn);
      oled_string_8x8(4,6,":");
      oled_num_8x8(5,6,sc);
    }
  }
}
