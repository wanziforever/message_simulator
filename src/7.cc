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
#include "udp_agent.hh"
#include "message.hh"
/*
extern UdpAgent *g_udp_agent;

Task::Task(std::string path)
{
  path_ = Utils::Path(path);
  path_ = Utils::Path(ConfigManager::getCWD()) + path_;
}

bool Task::processTask()
{
  // debugLog(NGB_TASK, "Task::processTask enter with path %s", path_.c_str());
  // struct dfs_msg msg;
  // bzero(&msg, sizeof(msg));
  // msg.op = DFS_INPUT_CHUNK;
  // strcpy(msg.inchk.file_path, "/Media/123456");
  // msg.inchk.idx = 1;
  // msg.inchk.size = 30;
  // msg.inchk.dst_cnt = 2;
  // const char *ip = ConfigManager::getDestAddress().c_str();
  // int port = 0;
  // sscanf(ConfigManager::getDestPort().c_str(), "%d", &port);
  // if (!g_udp_agent->sendMsg(ip, port, (char*)&msg, sizeof(msg))) {
  //   debugLog(NGB_TASK, "Task::processTask fail to send message");
  //   return;
  // }
  // debugLog(NGB_TASK, "Task::processTask send message successfully");

  if (items_.empty()) {
    debugLog(NGB_TASK, "Task::processTask no Item to run in this task");
    return true;
  }
  // go throuth the item list, and process one by one
  for (std::vector<RunItem>::iterator it = items_.begin();
       it != items_.end(); ++it ) {
    (RunItem *) it->processItem();
  }
}

bool Task::setupRunItems()
{
  char line[256];
  ifstream taskFile((path_.c_str()), ifstream::in);
  while (taskFile.getline(line, 256)) {
    std::string entry = std::string(line);
    entry = Utils::trim(entry);
    if (entry.size() && Utils::isComments(entry)) 
      continue;
    // the RunItem will accept the whole parameter line as input
    // detail format of parameter line will be parsed by specified
    // RunItem
    RunItem item(entry);
    items_.push_back(item);
  }
  return true;
}

*/