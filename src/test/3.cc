#include <iostream>
#include <unistd.h>
#include <sys/param.h>

main(int argc, char *argv[])
{
  char path[MAXPATHLEN];
  getcwd(path, MAXPATHLEN);
  std::cout << "current path:" << &path[0] << std::endl;
}
