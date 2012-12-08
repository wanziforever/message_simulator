#include <stdio.h>

int main(int argc, char *argv[])
{
  int b = 65;
  char a[10] = {0};
  int c = sprintf(a, "%d", b);
  printf("the return value of sprintf is %d\n", c);
  printf("the b is %s\n", a);
}
