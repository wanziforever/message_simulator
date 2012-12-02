#include "stdio.h"

int main(int argc, char *argv[])
{
  char *a = "1111130";
  int b = 0;
  int len = sscanf((char *)a, "%d", &b);
  printf("the len is %d, b is %d\n", len, b);
}
