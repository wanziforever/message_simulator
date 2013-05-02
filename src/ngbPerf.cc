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
#include <signal.h>
#include <stdlib.h>
#include "ngb_defs.hh"
#include "dictionary_manager.hh"
#include "config_manager.hh"
#include "task.hh"
#include "log_mgr.hh"
#include "log.hh"
#include "view.hh"
#include "load.hh"

//---------------------------------------------------------
// main
//---------------------------------------------------------

#ifdef UDP
#include "udp_agent.hh"
UdpAgent *g_udp_agent = 0;
#elif defined TCP
#include "tcp_agent.hh"
TcpAgent *g_tcp_agent = 0;
#elif defined TCPD
#include "tcpd_agent.hh"
TcpdAgent *g_tcpd_agent = 0;
#endif

const std::string LOG_FILE_NAME = "debuglog";
DictionaryManager *dictMgr = 0;

bool in_performance_environment = true;

bool SYSTEM_STOP = false;

void handleSignal();
void over();

View *view = 0;
Load *load = 0;
int main(int argc, char *argv[])
{
  LogMgr log_manager(LOG_FILE_NAME);
  NgbFormatTrace::init();
  ConfigManager::init("config.conf");
  debugLog(NGB_MAIN, "current working directory(%s),\n"
           " task(%s),"
           " dest_address(%s),"
           " dest_port(%d)",
           ConfigManager::getCWD().c_str(),
           ConfigManager::getTask().c_str(),
           ConfigManager::getDestAddress().c_str(),
           ConfigManager::getDestPort());
#ifdef UDP
  //std::cout <<" == UDP == "<< std::endl;
  g_udp_agent = new UdpAgent();
  g_udp_agent->init(ConfigManager::getLocalPort());
#elif defined TCP
  //std::cout <<" == TCP == ";fflush(stdout);
  g_tcp_agent = new TcpAgent();
  g_tcp_agent->init(ConfigManager::getLocalPort());
  if (!g_tcp_agent->tcpConnect(ConfigManager::getDestAddress().c_str(),
                            ConfigManager::getDestPort())) {
    std::cout << "fail to connect!" << std::endl;
    return false;
  }
  //std::cout << "connection established" << std::endl;
#elif defined TCPD
  //std::cout <<" == TCPD == ";fflush(stdout);
  g_tcpd_agent = new TcpdAgent();
  g_tcpd_agent->init(ConfigManager::getLocalPort());
  if (!g_tcpd_agent->tcpListen()) {
    std::cout << "fail to connect!" << std::endl;
    return false;
  }
  //std::cout << "connection established" << std::endl;
#endif
  dictMgr = new DictionaryManager;
  dictMgr->init("dictionary.xml");
  handleSignal();
  Task *task = new Task(ConfigManager::getTask());
  if (!task->init()) {
    debugLog(NGB_ERROR,
             "task initialization fail for %s", task->getPath().c_str());
    return -1;
  }
  load = new Load;
  load->start(task, ConfigManager::getExpectedTPS());
  //task->processTask();
  //
  //std::cout << "++--------- COMPLETED ------------++" << std::endl;
  view = new View;
  view->start();

  over();
  debugLog(NGB_MAIN, "program exit");
  return 0;
}

void over()
{
  if (dictMgr)     delete dictMgr;
  if (view)        delete view;
  if (load)        delete load;
  //if (task)        delete task;
#ifdef UDP
  if (g_udp_agent) delete g_udp_agent;
#elif defined TCP
  if (g_tcp_agent) delete g_tcp_agent;
#elif defined TCPD
  if (g_tcpd_agent) delete g_tcpd_agent;
#endif
}

void system_stop(int s)
{
  SYSTEM_STOP = true;
  sleep(2);
  over();
  debugLog(NGB_MAIN, "program stop");
  exit(1);
}

void handleSignal()
{
  signal(SIGINT, system_stop);
}
