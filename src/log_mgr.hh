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

// log manager class will manage the log print functionalities, it will
// control the detail log message, and where to print the log message

#ifndef NGB_LOG_MGR_H__
#define NGB_LOG_MGR_H__

#include <string>
#include <fstream>

class LogMgr
{
public:
  LogMgr(std::string logFileName);
  ~LogMgr();
  // print the error message
  static void errorMessage(const char *msg);
  // print the debug message
  static void debugMessage(char *msg);
  // host name for current host
  static std::string hostName_;
  static int log_file;
  static std::ofstream logFile;
  static bool isLogFileOpened;
};


#endif
