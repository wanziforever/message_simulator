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
#include "log.hh"

//---------------------------------------------------------
// main
//---------------------------------------------------------

UdpAgent *g_udp_agent = 0;
DictionaryManager *dictMgr = 0;

int main(int argc, char *argv[])
{
  NgbFormatTrace::init();
  ConfigManager::init("config.conf");

  debugLog(NGB_MAIN, "current working directory(%s),\n"
           " task(%s),"
           " dest_address(%s),"
           " dest_port(%s)",
           ConfigManager::getCWD().c_str(),
           ConfigManager::getTask().c_str(),
           ConfigManager::getDestAddress().c_str(),
           ConfigManager::getDestPort().c_str());
  g_udp_agent = new UdpAgent();
  int localPort = 0;
  sscanf(ConfigManager::getLocalPort().c_str(), "%d", &localPort);
  g_udp_agent->init(localPort);
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
}