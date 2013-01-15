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

#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "log.hh"
#include "run_item.hh"
#include "config_manager.hh"
#include "utils.hh"
#include "message.hh"
#include "udp_agent.hh"

char g_buf[COMMON_MSG_SIZE]= {0};
extern UdpAgent *g_udp_agent;

RunItem::RunItem(std::string line)
{
  setupRunItems(line);
}

bool RunItem::processItem()
{
  debugLog(NGB_RUN_ITEM, "RunItem::processItem enter...");
  display("++--------------------------------++");
  if (mode_ == "T") {
    debugLog(NGB_RUN_ITEM,
             "RunItem::processItem sending message");
    std::string progress = "TASK MODE (T) -- --> Sending message";
    display(progress);
    if (!sendMessage()) {
      debugLog(NGB_RUN_ITEM,
               "RunITem::processItem fail to send message");
      return false;
    }
  } else if (mode_ == "R") {
    debugLog(NGB_RUN_ITEM,
             "RunItem::processItem receiving message");
    std::string progress = "TASK MODE (R) -- --> Receiving message";
    display(progress);
    receiveMessage();
  } else {
    debugLog(NGB_RUN_ITEM,
             "RunItem::processItem unknown item mode(%s)",
             mode_.c_str());
    return false;
  }
  
  debugLog(NGB_RUN_ITEM, "RunItem::processItem exit...");
  return true;
}

void RunItem::display(std::string information)
{
  std::cout << information << std::endl;
}

bool RunItem::setupRunItems(std::string line)
{
  debugLog(NGB_RUN_ITEM, "RunItem::setupRunItems enter...");
  std::istringstream iss(line);
  iss >> mode_;
  std::string time;
  iss >> time;
  sscanf(time.c_str(), "%d", &timeOut_);
  std::string path;
  iss >> path;
  pathOfMessage_ = Utils::Path(ConfigManager::getCWD()) + Utils::Path(path);
  debugLog(NGB_RUN_ITEM,
           "RunItem::setupRunItems get parameters: mode(%s), timeOut(%d), "
           "pathOfMessage(%s)",
           mode_.c_str(), timeOut_, pathOfMessage_.c_str());
  debugLog(NGB_RUN_ITEM, "RunItem::setupRunItems exit...");
}

// the receiver side only accept such length of message, if the real
// message size is less, use 0 to fill the left space, logic couple
// to receiver system
#define FIX_MESSAGE_CONTENT_SIZE 1408

bool RunItem::sendMessage()
{
  debugLog(NGB_RUN_ITEM,
           "RunItem::sendMessage with msgfile(%s) enter",
           pathOfMessage_.toString().c_str());
  Message msg(pathOfMessage_.toString());
  if (!msg.init()) {
    debugLog(NGB_RUN_ITEM, "RunItem::sendMessage, fail to init message");
    return false;
  }
  msg.printRawEntry();
  display(msg.getDisplayData());
  memset(g_buf, 0, COMMON_MSG_SIZE);
  int len = msg.parseAppToRaw((char*)g_buf);
  // no matter what is the real size of the message to be sent, just set
  // fixed message length which is a logic couple from receiver system
  // msg.printDebug();
  // char bout[40] = {0};
  // Utils::binaryToAscii(g_buf, 10, (char*)bout);
  // debugLog(NGB_RUN_ITEM, "RunItem::sendMessage app to raw \n%s", bout);

  // ////// test code for parsing  message just finish encode //////////
  // Message msgR;
  // TODO: parseRawToApp to be the self handling of the raw data buffer
  // msgR.parseRawToApp(g_buf);
  // msgR.printDebug();
  // ////////////////////////// END ////////////////////////////////////
  
  len = FIX_MESSAGE_CONTENT_SIZE;
  //usleep(500000);
  g_udp_agent->sendMsg(ConfigManager::getDestAddress().c_str(),
                       ConfigManager::getDestPort(),
                       g_buf,
                       len);

  debugLog(NGB_RUN_ITEM, "RunItem::sendMessage exit...");
  return true;
}

bool RunItem::receiveMessage()
{
  debugLog(NGB_RUN_ITEM, "RunItem::receiveMessage enter...");
  // Message msg;
  // msg.init();
  // int len = msg.parseRawToApp((char*)g_buf);
  while (1) {
    if (g_udp_agent->getQueueSize() > 0) {
      break;
    }
    usleep(20000);
    debugLog(NGB_RUN_ITEM, "RunItem::receiveMessage waiting for message");
  }
  Message msg = g_udp_agent->receive();
  // TODO: parseRawToApp to be the self handling of the raw data buffer
  msg.parseRawToApp(msg.getRawPtr());
  msg.printDebug();
  display(msg.getDisplayData());
  debugLog(NGB_RUN_ITEM, "RunItem::receiveMessage exit...");
  return true;
}

void NGB_TEST(char *buf)
{
  debugLog(NGB_RUN_ITEM, "TEST");
  char test[5] = {0};
  strncpy(test, buf, 5);
  test[4] = '\0';
  debugLog(NGB_RUN_ITEM, "TEST first chars are %s", test);
}
