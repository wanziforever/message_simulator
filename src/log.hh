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

#ifndef NGB_LOG_H__
#define NGB_LOG_H__

#include "ngb_format_trace.hh"

extern const int NGB_TRACE_BASE;
extern const int NGB_ERROR_BASE;
extern const int NGB_ERROR;
extern const int NGB_MAIN;
extern const int NGB_DICT_MGR;
extern const int NGB_CONFIG_MGR;
extern const int NGB_TASK;
extern const int NGB_UDP_AGENT;
extern const int NGB_RUN_ITEM;
extern const int NGB_MESSAGE;
extern const int NGB_CONTAINER;
extern const int NGB_VALUE_PARSER;
extern const int NGB_TCP_AGENT;

#endif

