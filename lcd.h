#ifndef __LCD_H_
#define __LCD_H_

#define LCD_LINE_WIDTH 16
#define LCD_NUM_LINES 2

#define LCD_NUM_GPIO 7

#define LCD_DB7_GPIO (2*32+13) // P8_40, GPIO2_13
#define LCD_DB6_GPIO (2*32+12) // P8_39, GPIO2_12
#define LCD_DB5_GPIO (2*32+11) // P8_42, GPIO2_11
#define LCD_DB4_GPIO (2*32+10) // P8_41, GPIO2_10
#define LCD_E_GPIO (2*32+25) // P8_30, GPIO2_25
#define LCD_RS_GPIO (2*32+22) // P8_27, GPIO2_22
#define LCD_RW_GPIO (2*32+23) // P8_29, GPIO2_23

int lcd_init(void);
void lcd_clear(void);
void lcd_write(const char* format, ...);
void lcd_goto(int line, int pos);
void lcd_cleanup(void);

#endif /* __LCD_H_ */
