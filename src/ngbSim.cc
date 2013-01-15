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
// 
// This tool is designed to simulate the internal message for
// NGB project unit testing, the message body can be configured
// in the specified XML document.

#include <stdio.h>
#include "ngb_defs.hh"
#include "dictionary_manager.hh"
#include "config_manager.hh"
#include "task.hh"
#include "udp_agent.hh"
#include "log_mgr.hh"
#include "log.hh"

//---------------------------------------------------------
// main
//---------------------------------------------------------

const std::string LOG_FILE_NAME = "debuglog";
UdpAgent *g_udp_agent = 0;
DictionaryManager *dictMgr = 0;

int main(int argc, char *argv[])
{
  LogMgr log_manager(LOG_FILE_NAME);
  NgbFormatTrace::init();
  ConfigManager::init("config.conf");
  debugLog(NGB_MAIN, "current working directory(%s),\n"
           " task(%s),"
           " dest_address(%s),"
           " dest_port",
           ConfigManager::getCWD().c_str(),
           ConfigManager::getTask().c_str(),
           ConfigManager::getDestAddress().c_str(),
           ConfigManager::getDestPort());
  std::cout << "++--------------------------------++" << std::endl;
  std::cout << ConfigManager::getDisplayData() << std::endl;
  g_udp_agent = new UdpAgent();
  g_udp_agent->init(ConfigManager::getLocalPort());
  dictMgr = new DictionaryManager;
  dictMgr->init("dictionary.xml");
  Task *task = new Task(ConfigManager::getTask());
  if (!task->init()) {
    debugLog(NGB_MAIN,
             "task initialization fail for %s", task->getPath().c_str());
  }
  task->processTask();
  
  if (dictMgr)     delete dictMgr;
  if (task)        delete task;
  if (g_udp_agent) delete g_udp_agent;
  debugLog(NGB_MAIN, "program exit");
}
