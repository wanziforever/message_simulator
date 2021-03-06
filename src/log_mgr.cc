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

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "log_mgr.hh"

using namespace std;
const char *logFormat_tail = "\nEND OF REPORT ";
string LogMgr::hostName_;
int LogMgr::log_file = 0;
ofstream LogMgr::logFile;
bool LogMgr::isLogFileOpened = false;

string getCurrentTime()
{
  char timeb[64];
  long lt = time(0);
  struct tm *at = localtime((const long*) &lt);
  int yr = at->tm_year + 1900;
  sprintf(timeb, "%04d-%02d-%02d\t%02d:%02d:%02d",yr, at->tm_mon+1,
          at->tm_mday, at->tm_hour, at->tm_min, at->tm_sec);
  return string(timeb);
}

void removeNewLineSuffix(char *msg)
{
  int len = strlen(msg);
  int pos = len - 1;
  while(msg[pos] == '\n') {
    msg[pos] = 0;
  }
  return;
}

LogMgr::LogMgr(string logFileName)
{
  char name[64];
  if (gethostname(name, 64) != 0) {
    return;
  }
  hostName_.assign(name);
  logFile.open(logFileName.c_str());
  isLogFileOpened = true;
}

LogMgr::~LogMgr()
{
  logFile << flush;
  logFile.close();
  isLogFileOpened = false;
}

void LogMgr::errorMessage(const char *msg)
{
  logFile << string(msg) << endl;
}

void LogMgr::debugMessage(char *msg)
{
  if (!isLogFileOpened) {
    cout << "log file not opened" << endl;
    return;
  }
  removeNewLineSuffix(msg);
  string timestamp = getCurrentTime();
  string buf = string("+++  ") + hostName_ + string("\t") +
    string(timestamp) + string("\t") + string("DEBUG") +
    string("\n") + string("DEBUG LOG") + string("\t") +
    string(msg) + string(logFormat_tail) + string("+++  ");
  
  logFile << buf << "\n^A"<< endl;
}
