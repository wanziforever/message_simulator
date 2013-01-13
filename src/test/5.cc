#include <stdio.h>

typedef struct {
  short location;
//  short number;
  int padding;
} test;

int main()
{
  printf("the sizeof the test is %d\n", sizeof(test));
}
