// Message simulator - Hisense NGB project testing tool
// Copyright 2012 Hisense Inc.  All rights reserved.
//
// This software is free; you can redistribute it and/or modify it under
// the term of GNU Lesser General Public License as published by the free
// software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty, See the GNU
// Lesser General Public License for more details.
//
// Author: wangliang8@hisense.com (Denny Wang)
// History: created @2012/10/10

// the debug log will write to a debug log file, the file name is hard
// code to debuglog at the same directory which execute the binary,
// NOTE:
//   each execution will override the debug log file, so better not to
//   run two instance at the same directory

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "log_mgr.hh"
#include "ngb_format_trace.hh"

#define MAX_TRACE_FLAG 65535
// use char array to store the bigmap, so use bit number of char
const int BIT_NUM = 8; 
static unsigned char g_trace_flag_bitmap[MAX_TRACE_FLAG/8 + 1] = {0};
// const variable will use a internal linkage, so use extern before
extern const int NGB_TRACE_BASE = 2000;
extern const int NGB_ERROR_BASE = NGB_TRACE_BASE + 1000; // 3000
extern const int NGB_ERROR = NGB_ERROR_BASE + 1; // 3001
extern const int NGB_MAIN = NGB_TRACE_BASE + 1; // 2001
extern const int NGB_DICT_MGR = NGB_TRACE_BASE + 2; // 2002
extern const int NGB_CONFIG_MGR = NGB_TRACE_BASE + 3; // 2003
extern const int NGB_TASK = NGB_TRACE_BASE + 4; // 2004
extern const int NGB_UDP_AGENT = NGB_TRACE_BASE + 5; // 2005
extern const int NGB_RUN_ITEM = NGB_TRACE_BASE + 6; // 2006
extern const int NGB_MESSAGE = NGB_TRACE_BASE + 7; // 2007
extern const int NGB_CONTAINER = NGB_TRACE_BASE + 8; // 2008
extern const int NGB_VALUE_PARSER = NGB_TRACE_BASE + 9; // 2009
extern const int NGB_TCP_AGENT = NGB_TRACE_BASE + 10; // 2010
extern const int NGB_PERF_CONFIG = NGB_TRACE_BASE + 11; // 2011
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ADD TRACE NUMBER BEFORE THIS LINE, THE VALUE SHOULD NOT GREATER
// THAN THE MAX TRACE FLAG NUMBER
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
extern const int NGB_TRACE_END = MAX_TRACE_FLAG; // 65535

const int g_max_log_len = 2048;
// make sure the current directory has write permission
static const char *g_log_file_name = "debuglog";

// set the bitmap for the given flag
void setTraceFlag(int flag, bool value)
{
  if (flag >= MAX_TRACE_FLAG) {
    return;
  }
  if (value) { // set the bit field
    g_trace_flag_bitmap[flag / BIT_NUM] |= (1<<(flag % BIT_NUM));
  } else { // clear the bit field
    g_trace_flag_bitmap[flag / BIT_NUM] &= ~ (1<<(flag % BIT_NUM));
  }
}
// get the big field value for given flag
bool getTraceFlag(int flag)
{
  return (g_trace_flag_bitmap[flag / BIT_NUM] & (1<<(flag % BIT_NUM))) > 0;
}

// read from the trace_falg file, set trace flag for each line
bool parseTraceFlag()
{
  FILE *fd = 0;
  fd = fopen("trace_flag", "r");
  if (!fd) {
    LogMgr::errorMessage("Fail to open trace_flag file\n");
    return false;
  }
  char line[128];
  int flag = 0;
  while (fgets(line, sizeof(line), fd)) {
    sscanf(line, "%d", &flag);
    setTraceFlag(flag, true);
  }
  fclose(fd);
}

// check whether the given trace code is already opened
bool isTraceOpen(int code)
{
  return getTraceFlag(code);
}

void NgbFormatTrace::ngbDebugTrace(int code, char const *fmt, ...)
{
  // not print anything if the trace is not opened
  if(!isTraceOpen(code)) {
    return;
  }
  char traceBuf[g_max_log_len];
  va_list args;
  va_start(args, fmt);
  if (vsnprintf(traceBuf, (size_t) g_max_log_len, fmt, args) < 0) {
    LogMgr::errorMessage("ngbFormatTrace fail to sprintf string\n");
    return;
  }
  va_end(args);
  char detailTraceBuf[g_max_log_len + 512];
  sprintf(detailTraceBuf,
          "FileName: %s line number: %d trace: %d\n",
          fileName_, lineNum_, code);
  strcat(detailTraceBuf,traceBuf);
  LogMgr::debugMessage(detailTraceBuf);
}

void NgbFormatTrace::init()
{
  parseTraceFlag();
}
