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
#endif
