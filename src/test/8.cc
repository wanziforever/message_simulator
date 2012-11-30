#include "stdio.h"
#include "iostream"
#include "string.h"

using namespace std;
main()
{
  string a = "";
  const char *b = a.c_str();
  printf("the b value is %s, length is %d\n", b, strlen(b));
}
