#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "oled.h"

const uint8_t oled_init_seq [] PROGMEM =
{
  0xAE,         //display OFF (sleep mode)
  0x20, 0x00,   //set memory addressing mode to horizontal addressing mode 
  0xB0,         //set page start address for page addressing mode, 0-7
  0xC8,         //set COM output scan direction
  0x00,         //---set low column address
  0x10,         //---set high column address
  0x40,         //--set start line address
  0x81, 0x3F,   //set contrast control register
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
  oled_send_byte(0xb0 + y);
  oled_send_byte(((x & 0xf0) >> 4) | 0x10);
  oled_send_byte((x & 0x0f) | 0x01);
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
    oled_tx_stop();
  }
}
