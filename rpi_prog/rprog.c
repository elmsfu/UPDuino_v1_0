
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <bcm2835.h>

#include "RaspberryPi_Prog.c"
//#include "RGB_LED_BLINK.c"

void usage() {
  printf("Usage: rtest <binfile>\n");

  exit(1);
}

int main(int argc, char **argv) {
  int retval = 1;

//bcm2835_set_debug(1);
  if (!bcm2835_init())
    return 2;

  if (argc<2)
    usage();

  FILE* fbitmap = fopen(argv[1], "r");
  if (NULL == fbitmap) {
    printf("failed to open %s\n", argv[1]);
    return 1;
  }

  struct stat bit_stat;
  stat(argv[1], &bit_stat);
  size_t bit_size = bit_stat.st_size;
  printf("bitmap size: %d\n", bit_size);

  uint8_t* bitmap = malloc(bit_size);
  fread(bitmap, bit_size, 1, fbitmap);

  retval = cfg(bitmap, bit_size);

  free(bitmap);
  return retval;
}
