#include "stdio.h"

int main(int argc, char *argv[])
{
  int fd;
  if (argc == 2) {
    printf("invalid argument\n");
    exit(1);
  }
  if ((fd = fopen(argv[1]), "r") == -1) {
    
  }
  
}
