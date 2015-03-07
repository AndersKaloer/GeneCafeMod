#include "lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define LCD_DB7 6
#define LCD_DB6 5
#define LCD_DB5 4
#define LCD_DB4 3
#define LCD_E 2
#define LCD_RW 1
#define LCD_RS 0

FILE *fds[LCD_NUM_GPIO];

void set_gpio(FILE *fd, uint8_t val) {
  fseek(fd, 0, SEEK_SET);
  fprintf(fd, "%d", val & 1);
  fflush(fd);
}

int gpio_open(int gpio) {
  FILE *fd;
  fd = fopen("/sys/class/gpio/export", "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open export\n");
    return -1;
  }
  fprintf(fd, "%d", gpio);
  fclose(fd);
  char file[50];
  sprintf(file, "/sys/class/gpio/gpio%d/direction", gpio);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open direction\n");
    return -1;
  }
  fprintf(fd, "low"); // OUTPUT low
  fclose(fd);
  return 0;
}

int gpio_close(int gpio) {
  FILE *fd;
  fd = fopen("/sys/class/gpio/unexport", "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open unexport\n");
    return -1;
  }
  fprintf(fd, "%d", gpio);
  fclose(fd);
  return 0;
}

int init_gpio(void) {
  if(gpio_open(LCD_DB7_GPIO)) {
    goto err_gpio;
  }
  if(gpio_open(LCD_DB6_GPIO)) {
    goto err_gpio;
  }
  if(gpio_open(LCD_DB5_GPIO)) {
    goto err_gpio;
  }
  if(gpio_open(LCD_DB4_GPIO)) {
    goto err_gpio;
  }
  if(gpio_open(LCD_E_GPIO)) {
    goto err_gpio;
  }
  if(gpio_open(LCD_RW_GPIO)) {
    goto err_gpio;
  }
  if(gpio_open(LCD_RS_GPIO)) {
    goto err_gpio;
  }
  
  char file[50];
  sprintf(file, "/sys/class/gpio/gpio%d/value", LCD_DB7_GPIO);
  fds[LCD_DB7] = fopen(file, "w");
  if(fds[LCD_DB7] == NULL) {
    fprintf(stderr, "ERROR: fd_lcd_db7\n");
    goto err_db7;
  }
  sprintf(file, "/sys/class/gpio/gpio%d/value", LCD_DB6_GPIO);
  fds[LCD_DB6] = fopen(file, "w");
  if(fds[LCD_DB6] == NULL) {
    fprintf(stderr, "ERROR: fd_lcd_db6\n");
    goto err_db6;
  }
  sprintf(file, "/sys/class/gpio/gpio%d/value", LCD_DB5_GPIO);
  fds[LCD_DB5] = fopen(file, "w");
  if(fds[LCD_DB5] == NULL) {
    fprintf(stderr, "ERROR: fd_lcd_db5\n");
    goto err_db5;
  }
  sprintf(file, "/sys/class/gpio/gpio%d/value", LCD_DB4_GPIO);
  fds[LCD_DB4] = fopen(file, "w");
  if(fds[LCD_DB4] == NULL) {
    fprintf(stderr, "ERROR: fd_lcd_db4\n");
    goto err_db4;
  }
  sprintf(file, "/sys/class/gpio/gpio%d/value", LCD_E_GPIO);
  fds[LCD_E] = fopen(file, "w");
  if(fds[LCD_E] == NULL) {
    fprintf(stderr, "ERROR: fd_lcd_e\n");
    goto err_e;
  }
  sprintf(file, "/sys/class/gpio/gpio%d/value", LCD_RW_GPIO);
  fds[LCD_RW] = fopen(file, "w");
  if(fds[LCD_RW] == NULL) {
    fprintf(stderr, "ERROR: fd_lcd_rw\n");
    goto err_rw;
  }
  sprintf(file, "/sys/class/gpio/gpio%d/value", LCD_RS_GPIO);
  fds[LCD_RS] = fopen(file, "w");
  if(fds[LCD_RS] == NULL) {
    fprintf(stderr, "ERROR: fd_lcd_rs\n");
    goto err_rs;
  }
  return 0;
  
 err_rs:
  fclose(fds[LCD_RW]);
 err_rw:
  fclose(fds[LCD_E]);
 err_e:
  fclose(fds[LCD_DB4]);
 err_db4:
  fclose(fds[LCD_DB5]);
 err_db5:
  fclose(fds[LCD_DB6]);
 err_db6:
  fclose(fds[LCD_DB7]);
 err_db7:
  gpio_close(LCD_DB7_GPIO);
  gpio_close(LCD_DB6_GPIO);
  gpio_close(LCD_DB5_GPIO);
  gpio_close(LCD_DB4_GPIO);
  gpio_close(LCD_E_GPIO);
  gpio_close(LCD_RW_GPIO);
  gpio_close(LCD_RS_GPIO);
 err_gpio:
  return -1;
}

void send_gpio(uint16_t val, uint8_t initializing) {
  int i;
  // RS RW DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
  set_gpio(fds[LCD_RS], (val >> 9) & 1);
  set_gpio(fds[LCD_RW], (val >> 8) & 1);
  for(i = 0; i < 4; i++) {
    set_gpio(fds[LCD_DB4+i], (val >> (4+i)) & 1);
  }
  usleep(10);
  set_gpio(fds[LCD_E], 1);
  usleep(100);
  set_gpio(fds[LCD_E], 0);
  usleep(10);
  if(!initializing) {
    for(i = 0; i < 4; i++) {
      set_gpio(fds[LCD_DB4+i], (val >> i) & 1);
    }
    usleep(10);
    set_gpio(fds[LCD_E], 1);
    usleep(100);
    set_gpio(fds[LCD_E], 0);
    usleep(10);
  }
  for(i = 0; i < LCD_NUM_GPIO; i++) {
    set_gpio(fds[i], 0);
  }
  usleep(100);
}

void lcd_clear(void) {
  uint16_t val = 0b0000000001;
  send_gpio(val, 0);
}

void lcd_on(void) {
  // Display on/off control
  uint16_t val = 0b0000001100;
  send_gpio(val, 0);
  // Entry mode set
  val = 0b0000000110;
  send_gpio(val, 0);
}

int lcd_init(void) {
  if(init_gpio()) {
    return -1;
  }
  // Send Function Set command
  // 4-bit mode
  uint16_t val = 0b0000100100;
  send_gpio(val, 1);
  lcd_on();
  lcd_clear();
  return 0;
}

void lcd_cleanup(void) {
  // Disable 4-bit mode
  uint16_t val = 0b0000110000;
  send_gpio(val, 0);
  int i;
  for(i = 0; i < LCD_NUM_GPIO; i++) {
    fclose(fds[i]);
  }
  gpio_close(LCD_DB7_GPIO);
  gpio_close(LCD_DB6_GPIO);
  gpio_close(LCD_DB5_GPIO);
  gpio_close(LCD_DB4_GPIO);
  gpio_close(LCD_E_GPIO);
  gpio_close(LCD_RW_GPIO);
  gpio_close(LCD_RS_GPIO);
}

void lcd_goto(int line, int pos) {
  if(line < LCD_NUM_LINES && pos < LCD_LINE_WIDTH) {
    uint8_t ddram_addr = 0;
    if(LCD_NUM_LINES == 2) {
      switch(line) {
      case 0:
        ddram_addr = 0x00;
        break;
      case 1:
        ddram_addr = 0x40;
        break;
      }
    } else if(LCD_NUM_LINES == 4) {
      switch(line) {
      case 0:
        ddram_addr = 0x00;
        break;
      case 1:
        ddram_addr = LCD_LINE_WIDTH;
        break;
      case 2:
        ddram_addr = 0x40;
        break;
      case 3:
        ddram_addr = 0x40+LCD_LINE_WIDTH;
        break;
      }
    }
    ddram_addr += pos;
    // Goto pos
    uint16_t val = (0b0010000000 | (ddram_addr & 0x7F));
    send_gpio(val, 0);
  }
}

void lcd_write(const char *format, ...) {
  static char str_buff[LCD_LINE_WIDTH*LCD_NUM_LINES+1];
  va_list args;
  va_start(args, format);
  vsnprintf(str_buff, LCD_LINE_WIDTH*LCD_NUM_LINES+1, format, args);
  va_end(args);
  uint16_t val;
  int i;
  for(i = 0; i < strlen(str_buff); i++) {
    val = (0b10 << 8);
    val |= str_buff[i];
    send_gpio(val, 0);
  }
}
