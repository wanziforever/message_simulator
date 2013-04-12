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

Task::Task(std::string path)
{
  path_ = Utils::Path(path);
  path_ = Utils::Path(ConfigManager::getCWD()) + path_;
}

// only setup the run items is called during initialization currently
bool Task::init()
{
  return setupRunItems();
}

std::string Task::getPath()
{
  return path_.toString();
}

bool Task::processTask()
{
  if (items_.empty()) {
    debugLog(NGB_TASK, "Task::processTask no Item to run in this task");
    return true;
  }
  // go throuth the item list, and process one by one
  for (std::vector<RunItem>::iterator it = items_.begin();
       it != items_.end(); ++it ) {
    if (!(*it).processItem()) {
      debugLog(NGB_TASK, "Task::processTask processItem fail");
      return false;
    }
  }
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
  debugLog(NGB_TASK, "Task::setupRunItems exit");
  return true;
}

