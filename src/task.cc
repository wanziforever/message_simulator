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

#include <stdint.h>
#include <string.h>
#include <fstream>
#include <sys/param.h>
#include "log.hh"
#include "task.hh"
#include "config_manager.hh"
#include "message.hh"
#include "perf_config_manager.hh"
#include "utils.hh"
#include <sys/time.h>


Task::Task(std::string path)
{
  path_ = Utils::Path(path);
  path_ = Utils::Path(ConfigManager::getCWD()) + path_;
  sendCounter_ = 0;
  receivedCounter_ = 0;
}

// only setup the run items is called during initialization currently
bool Task::init()
{
  if (!setupRunItems()) {
      debugLog(NGB_TASK, "Task::init fail to setupRunItems");
      return false;
  }
  
  if (!setupPerformanceData()) {
    debugLog(NGB_TASK, "Task::init fail to setupPerformanceData");
    return false;
  }
  return true;
}

std::string Task::getPath()
{
  return path_.toString();
}

// processTask will return the duration for processing on task in usec
unsigned long long Task::processTask()
{
  unsigned long long start = 0, finish = 0;
  start = Utils::getTimeOfDayinUsec();
  
  if (items_.empty()) {
    debugLog(NGB_TASK, "Task::processTask no Item to run in this task");
    return 0;
  }
  debugLog(NGB_TASK, "Task::processTask before process %s", toString().c_str());
  // go throuth the item list, and process one by one
  for (std::vector<RunItem>::iterator it = items_.begin();
       it != items_.end(); ++it ) {
    if (!(*it).processItem()) {
      debugLog(NGB_ERROR, "Task::processTask processItem fail");
      return -1;
    }
    if ((*it).getMode() == "T") {
      sendCounter_ = (*it).getCounter();
    } else if ((*it).getMode() == "R") {
      receivedCounter_ = (*it).getCounter();
    }
  }

  finish = Utils::getTimeOfDayinUsec();
  return (finish - start);
}

bool Task::setupRunItems()
{
  debugLog(NGB_TASK, "Task::setupRunItems enter");
  char line[256];
  std::ifstream taskFile((path_.c_str()), std::ifstream::in);
  //TODO report the file not exist error
  while (taskFile.getline(line, 256)) {
    std::string entry = std::string(line);
    entry = Utils::trim(entry);
    if (!entry.size() || Utils::isComments(entry)) 
      continue;
    // the RunItem will accept the whole parameter line as input
    // detail format of parameter line will be parsed by specified
    // RunItem
    RunItem item(entry);
    items_.push_back(item);
  }
  debugLog(NGB_TASK,"Task::setupRunItems print %s", toString().c_str());
  debugLog(NGB_TASK, "Task::setupRunItems exit");
  return true;
}

bool Task::setupPerformanceData()
{
  debugLog(NGB_TASK, "Task::setupPerformanceData enter");
  for (std::vector<RunItem>::iterator ite = items_.begin();
       ite != items_.end(); ite++) {
    debugLog(NGB_TASK, "Task::setupPerformanceData %s",
             (*ite).getMode().c_str());
    (*ite).setupPerformanceData();
    debugLog(NGB_TASK, "Task::setupPerformanceData new %s",
             (*ite).getMode().c_str());
  }
  debugLog(NGB_TASK, "Task::setupPerformanceData exit");
  return true;
}

unsigned long long Task::getMsgReceived()
{
  return receivedCounter_;
}

unsigned long long Task::getMsgSend()
{
  return sendCounter_;
}

std::string Task::toString()
{
  std::string ret;
  for (std::vector<RunItem>::iterator ite = items_.begin();
       ite != items_.end(); ite++) {
    ret += (*ite).toString() + "\n";
  }
  return ret;
}
