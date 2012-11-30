#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define false 0
#define true 1

//#define MAX_TRACE_FLAG 65535
#define MAX_TRACE_FLAG 8
#define BIT_NUM (8)
unsigned char g_trace_flag_bitmap[MAX_TRACE_FLAG/8 + 1] = {0};

void printBigMap()
{
  std::cout << "---" << (int) (g_trace_flag_bitmap[0]) << std::endl;
  std::cout << "---" << (int) (g_trace_flag_bitmap[1]) << std::endl;
}
// set the bitmap for the given flag
void setTraceFlag(int flag, bool value)
{
  if (value) { // set the bit field
    std::cout << "goint to set the falg: " << flag / BIT_NUM << std::endl;
    std::cout << "going to set the flag: "
              << (int)g_trace_flag_bitmap[flag / BIT_NUM] << std::endl;
    g_trace_flag_bitmap[flag / BIT_NUM] |= (1<<(flag % BIT_NUM));
    std::cout << "going to set the flag: "
              << (int)g_trace_flag_bitmap[flag / BIT_NUM] << std::endl;
  } else { // clear the bit field
    g_trace_flag_bitmap[flag / BIT_NUM] &= ~ (1<<(flag % BIT_NUM));
  }
}
// get the big field value for given flag
bool getTraceFlag(int flag)
{
  std::cout << "get trace flag: " << (int) g_trace_flag_bitmap[flag/BIT_NUM] << std::endl;
  std::cout << "get trace flag: "
            << (int)(((g_trace_flag_bitmap[flag / BIT_NUM]) & 8)>0)
            << std::endl;
  return (g_trace_flag_bitmap[flag / BIT_NUM] & (1<<(flag % BIT_NUM)) > 0);
}

// read from the trace_falg file, set trace flag for each line
bool parseTraceFlag()
{
  FILE *fd = 0;
  fd = fopen("trace_flag", "r");
  if (!fd) {
    std::cout << "fail to open trace_flag file\n" << std::endl;
  }
  char line[128];
  int flag = 0;
  while (fgets(line, sizeof(line), fd)) {
    sscanf(line, "%d", &flag);
    std::cout << "get trace " << flag << std::endl;
    setTraceFlag(flag, true);
  }
  fclose(fd);
}

main(int argc, char *argv[])
{
  int flag = 3;
  printBigMap();
  std::cout << "before set the flag:" << std::endl;
  std::cout << getTraceFlag(flag) << std::endl;
  std::cout << "set the flag..." << std::endl;
  setTraceFlag(flag, true);
  std::cout << "after set the flag" << std::endl;
  std::cout << getTraceFlag(flag) << std::endl;
  printBigMap();
}