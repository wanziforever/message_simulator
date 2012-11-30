#include <fstream.h>

void binaryToHexForOneChar(char);

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cout << "invalid argument" << std::endl;
    return -1;
  }
  ofstream in(argv[1], ios::in|ios::binary|ios::nocreate);
  if(!in) {
    std::cout << "fail to open file" << argv[1] << std::endl;
    return -1;
  }
  int numOfCharToRead = 10
  int i = 1;
  char a;
  for (; !in.eof() && i <= numOfCharToRead; i++) {
    in.read((charr*)&a, sizeof(char));
    binaryToHexForOneChar(a);
  }
  std::cout << std::endl;
  return 0;
}

void binaryToHexForOneChar(char c)
{
  char input_char = c;
  char hig = (input_char >> 4) & 0x0f;
  char low = (input_char) &  0x0f;
  std::cout << hig << low;
}
