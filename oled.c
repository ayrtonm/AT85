#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "oled.h"
#include "font8x8.h"

const uint8_t oled_init_seq [] PROGMEM =
{
  0xAE,         //display OFF (sleep mode)
  0x20, 0x00,   //set memory addressing mode to horizontal addressing mode 
  0xB0,         //set page start address for page addressing mode, 0-7
  0xC8,         //set COM output scan direction
  0x00,         //---set low column address
  0x10,         //---set high column address
  0x40,         //--set start line address
  0x81, 0x0F,   //set contrast control register
  0xA1,         //set segment re-map. A0=address mapped; A1=address 127 mapped
  0xA6,         //set display mode. A6=normal; A7=inverse
  0xA8, 0x3F,   //set multiplex ratio, 1-64
  0xA4,         //output RAM to display. A4=output follows RAM; A5=output ignores RAM
  0xD3, 0x00,   //set display offset. 00 = no offset
  0xD5,         //--set display clock divide ratio/oscillator frequency
  0xF0,         //--set divide ratio
  0xD9, 0x22,   //set pre-charge period
  0xDA, 0x12,   //set com pins hardware configuration
  0xDB,         //--set vcomh
  0x20,         //0x20,0.77*Vcc
  0x8D, 0x14,   //set DC-DC enable
  0xAF          //display ON (normal mode)
};

void oled_init(void)
{
  DDRB |= (1 << OLED_SDA);
  DDRB |= (1 << OLED_SCL);
  uint8_t i;
  for (i = 0;i < sizeof(oled_init_seq);i++)
  {
    oled_send_command(pgm_read_byte(&oled_init_seq[i]));
  }
}
void oled_tx_start(void)
{
  PORTB |= (1 << OLED_SCL);
  PORTB |= (1 << OLED_SDA);
  PORTB &= ~(1 << OLED_SDA);
  PORTB &= ~(1 << OLED_SCL);
}
void oled_tx_stop(void)
{
  PORTB &= ~(1 << OLED_SCL);
  PORTB &= ~(1 << OLED_SDA);
  PORTB |= (1 << OLED_SCL);
  PORTB |= (1 << OLED_SDA);
}
void oled_send_byte(uint8_t byte)
{
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    if ((byte << i) & 0x80) PORTB |= (1 << OLED_SDA);
    else PORTB &= ~(1 << OLED_SDA);
    PORTB |= (1 << OLED_SCL);
    PORTB &= ~(1 << OLED_SCL);
  }
  PORTB |= (1 << OLED_SDA);
  PORTB |= (1 << OLED_SCL);
  PORTB &= ~(1 << OLED_SCL);
}
void oled_send_command_start(void)
{
  oled_tx_start();
  oled_send_byte(OLED_SA);//i2c slave address
  oled_send_byte(0x00);//write command
}
void oled_send_command(uint8_t cmd)
{
  oled_send_command_start();
  oled_send_byte(cmd);
  oled_tx_stop();
}
void oled_send_data_start(void)
{
  oled_tx_start();
  oled_send_byte(OLED_SA);
  oled_send_byte(0x40); //write data
}
void oled_setpos(uint8_t x, uint8_t y)
{
  oled_send_command_start();
  y %= 8;
  oled_send_byte(0xb0 + y);
  oled_send_byte(x & 0x0f);
  oled_send_byte(((x & 0xf0) >> 4) | 0x10);
  oled_tx_stop();
}
void oled_fillscreen(uint8_t fill)
{
  uint8_t m;
  for (m = 0; m < 8; m++)
  {
    oled_send_command(0xb0+m);//page0-page1
    oled_send_command(0x00);//low column start address
    oled_send_command(0x10);//high column start address
    oled_send_data_start();
    uint8_t n;
    for (n = 0; n < 128; n++)
    {
      oled_send_byte(fill);
    }
  }
}
void oled_string_8x8(uint8_t x, uint8_t y, const char ch[])
{
  uint8_t c, j = 0;
  while (ch[j] != '\0')
  {
    c = ch[j] - 32;
    if (x > 120)
    {
      x = 0;
      y++;
    }
    oled_setpos(x,y);
    oled_send_data_start();
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
      oled_send_byte(pgm_read_byte(&oled_font8x8[c*8+i]));
    }
    oled_tx_stop();
    x += 8;
    j++;
  }
}

//buffer for ascii representation of numbers
//for 16 bit number max decimal digits is 5 + 1 for '\0' string terminator
char oled_num_buffer[6];

void oled_num_8x8(uint8_t x, uint8_t y, uint16_t num)
{
  oled_num_buffer[5] = '\0';//terminate string
  uint8_t digits = uint_to_ascii(num,oled_num_buffer);
  oled_string_8x8(x,y,digits + oled_num_buffer);
}

uint8_t uint_to_ascii(uint16_t num, char *buffer)
{
  const unsigned short powers[] = {10000u, 1000u, 100u, 10u, 1u};
  char digit;
  uint8_t digits = 4; 
  uint8_t pos;
  for (pos = 0; pos < 5; pos++)
  {
    digit = 0;
    while (num >= powers[pos])
    {
      digit++;
      num -= powers[pos];
    }
    if (digits == 4)
    {
      if (digit == 0)
      {
        if (pos < 4)
        {
          digit = 0;//"-16" for space ' ', use "0" for zero, "-3" for minus
        }
      }
      else 
      {
        digits = pos;
      }
    }
    buffer[pos] = digit + '0';
  }
  return digits;
}
