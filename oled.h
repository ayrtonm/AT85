#ifndef OLED_H
#define OLED_H

#define OLED_SCL    PB3
#define OLED_SDA    PB4
#define OLED_SA     0x78  //i2c slave address

void oled_init(void);
void oled_tx_start(void);
void oled_tx_stop(void);
void oled_send_byte(uint8_t byte);
void oled_send_command_start(void);
void oled_send_command(uint8_t cmd);
void oled_send_data_start(void);
void oled_setpos(uint8_t x, uint8_t y);
void oled_fillscreen(uint8_t fill);
void oled_string_8x8(uint8_t x, uint8_t y, const char ch[]);
void oled_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
void oled_num_8x8(uint8_t x, uint8_t y, uint16_t num);
uint8_t uint_to_ascii(uint16_t, char *);
#endif
