#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define RX_BUF_LEN 4

int fd;

static int max31855_check_failures(uint32_t val) {
  int retval = 0;
  
  if((val & 0x10000)) {
    /* Some error */
    if(val & 0x1) {
      /* Open circuit fault */
      //fprintf(stderr, "max31855: read failure: open circuit\n");
      retval = -1;
    }
    if(val & 0x2) {
      /* Short to GND fault */
      //fprintf(stderr, "max31855: read failure: short to gnd\n");
      retval = -2;
    }
    if(val & 0x4) {
      /* Short to Vcc fault */
      //fprintf(stderr, "max31855: read failure: short to vcc\n");
      retval = -3;
    }    
  }
  return retval;
}

float max31855_get_temp(int *err) {
  struct spi_ioc_transfer xfer;
  unsigned char rx_buf[RX_BUF_LEN];
  int status;
  uint32_t read_val;
  memset(&xfer, 0, sizeof(struct spi_ioc_transfer));
  xfer.rx_buf = (unsigned long)rx_buf;
  xfer.len = RX_BUF_LEN;

  status = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
  if(status < 0) {
    *err = 1;
    return 0.0;
  }
  read_val = (((uint32_t)rx_buf[0]) << 24)
    | (((uint32_t)rx_buf[1]) << 16)
    | (((uint32_t)rx_buf[2]) << 8)
    | (((uint32_t)rx_buf[3]));
  float therm_val_float = 0;
  if(!max31855_check_failures(read_val)) {
    /* Bits 31-16 are thermocoupler readings */
    int16_t therm_val = (read_val >> 16) & 0xFFFC;
    therm_val = (therm_val >> 2);
    /* Remove padding and fault bits */
    /* Cast to signed int to preserve sign */
    therm_val_float = therm_val*0.25;
  }
  
  return therm_val_float;
}

int max31855_init(char *sys_file) {
  fd = open(sys_file, O_RDWR);
  if(fd < 0) {
    fprintf(stderr, "open");
    return -1;
  }
  return 0;
}

void max31855_cleanup(void) {
  if(close(fd)) {
    fprintf(stderr, "close");
  }
}
