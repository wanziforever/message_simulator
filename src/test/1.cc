#include <fstream>
#include <iostream>
#include <stdio.h>

void binaryToHexForOneChar(char);

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cout << "invalid argument" << std::endl;
    return -1;
  }
  std::ifstream in(argv[1], std::ios::in|std::ios::binary);
  if(!in) {
    std::cout << "fail to open file" << argv[1] << std::endl;
    return -1;
  }
  int numOfCharToRead = 10;
  int i = 1;
  char a;
  for (; !in.eof() && i <= numOfCharToRead; i++) {
    in.read((char*)&a, sizeof(char));
    binaryToHexForOneChar(a);
  }
  std::cout << std::endl;
  return 0;
}

char toHex(char input)
{
  if (input > 0 && input < 10) {
    return ('0' + input);
  } else if (input >=10 && input < 16) {
    return ('a' + input - 10);
  } else {
    return '0';
  }
}

void binaryToHexForOneChar(char c)
{
  char input_char = c;
  char hig = (input_char >> 4) & 0x0f;
  char low = (input_char) &  0x0f;
  hig = toHex(hig);
  low = toHex(low);
  std::cout << hig << low;
  //printf("hig is %d, low is %d\n", hig, low);
}
