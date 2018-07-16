//The following C code is based Raspberry PI.
//Because the slave select pin of Raspberry SPI interface cannot be controlled by software independently.
//4 GPIOs in Bitbang mode are used to imitate the SPI timing.
//“JDI_LCD_bitmap.c” in the source file include C array converted from hex file generated in iCECube. A sample file is attached.
//User can call “cfg()” in the main procedure to finish the configuration.

#include "bcm2835.h"


#define SDO RPI_BPLUS_GPIO_J8_35
#define SCLK RPI_BPLUS_GPIO_J8_36
#define CSN RPI_BPLUS_GPIO_J8_37
#define SDI RPI_BPLUS_GPIO_J8_38
#define CRESETB RPI_BPLUS_GPIO_J8_40
#define CDONE RPI_BPLUS_GPIO_J8_15

// magic number of bytes to send after CDONE is set
#define POST_CDONE_SEND     5
#define CDONE_IT_SLEEP_USEC 100*1000
#define CDONE_ITS           100

void assert_sdo () {
       bcm2835_gpio_write(SDO, HIGH);
}

void dessert_sdo () {
    bcm2835_gpio_write(SDO, LOW);
}

void assert_sclk () {
    bcm2835_gpio_write(SCLK, HIGH);
}

void dessert_sclk () {
    bcm2835_gpio_write(SCLK, LOW);
}

void sendbyte (char data) {
  char temp;
  int  i;
  temp = data;
  for (i=0;i<8;i++) {
    if ((temp & 128)>0)
      assert_sdo();
    else
      dessert_sdo();

    assert_sclk();
    dessert_sclk();
    temp = temp <<1;
  }
}

void assert_ss () {
    bcm2835_gpio_write(CSN, LOW);
}

void dessert_ss () {
    bcm2835_gpio_write(CSN, HIGH);
}

void init_spi () {
   bcm2835_gpio_fsel(CSN, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(SCLK, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(CRESETB, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(CDONE, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel(SDO, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(SDI, BCM2835_GPIO_FSEL_INPT);

   //bcm2835_gpio_write(CSN, HIGH);
   //bcm2835_gpio_write(SCLK, HIGH);
   //bcm2835_gpio_write(SDO, LOW);
}

int cfg(uint8_t* bitmap, size_t len) {
  size_t i;

  init_spi();
  printf("Program is running!\n");

  assert_ss();

  char pdone = 0;

  bcm2835_gpio_write(CRESETB, LOW);
  bcm2835_delay(10);

  for (int ii=0; ii<CDONE_ITS; ii++) {
    if (bcm2835_gpio_lev(CDONE) == LOW) {
      break;
    }
    usleep(CDONE_IT_SLEEP_USEC);
  }

  if (bcm2835_gpio_lev(CDONE) == HIGH) {
    printf("timeout waiting for CDONE after %f sec\n", (CDONE_ITS*CDONE_IT_SLEEP_USEC)/1e6);
    return 1;
  }

  printf("CDONE is low!\n");
  bcm2835_gpio_write(CRESETB, HIGH);

  delay(2000);
  //bcm2835_gpio_write(CRESETB, LOW);
  //delay(2000);

  bcm2835_delay(1);

  for (i=0; i<2*len; i++) {
    printf("running 1\n");

    if (i < len) {
      sendbyte(bitmap[i]);
      printf("running 2\n");
    } else {
      printf("running =3\n");
      sendbyte(0);
    }

    if ((pdone==0) && (bcm2835_gpio_lev(CDONE)==HIGH)) {
      printf("CDONE is High! End of Configuration!\n");
      pdone = 1;
      for (int jj=0; jj<POST_CDONE_SEND; jj++) {
        sendbyte(0);
      }
      break;
    }

  }

  if (pdone==0) {
    printf("CDONE is Low! Configuration Error!\n");
  }

  dessert_ss();

  return (pdone==0);
}
