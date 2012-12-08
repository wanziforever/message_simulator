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
#include <stdio.h>
#include "log.hh"
#include "run_item.hh"
#include "config_manager.hh"
#include "utils.hh"
#include "message.hh"

char g_buf[MAX_MESSAGE_SIZE]= {0};

RunItem::RunItem(std::string line)
{
  setupRunItems(line);
}

void RunItem::processItem()
{
  debugLog(NGB_RUN_ITEM, "RunItem::processItem enter...");
  if (mode_ == "T") {
    debugLog(NGB_RUN_ITEM,
             "RunItem::processItem sending message");
    if (!sendMessage()) {
      debugLog(NGB_RUN_ITEM,
               "RunITem::processItem fail to send message");
      return;
    }
  } else if (mode_ == "R") {
    debugLog(NGB_RUN_ITEM,
             "RunItem::processItem receiving message");
    receiveMessage();
  } else {
    debugLog(NGB_RUN_ITEM,
             "RunItem::processItem unknown item mode(%s)",
             mode_.c_str());
    return;
  }
  
  debugLog(NGB_RUN_ITEM, "RunItem::processItem exit...");
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

bool RunItem::sendMessage()
{
  debugLog(NGB_RUN_ITEM, "RunItem::sendMessage enter...");
  Message msg(pathOfMessage_.toString());
  if (!msg.init()) {
    debugLog(NGB_RUN_ITEM, "RunItem::sendMessage, fail to init message");
    return false;
  }
  msg.print();
  int len = msg.parseAppToRaw((char*)g_buf);
  msg.print();
  char *raw_readable = new char[len * 2 + 1];
  // sendto(buf);
  Utils::binaryToAscii(g_buf, len, raw_readable);
  raw_readable[len*2] = '\0';
  debugLog(NGB_RUN_ITEM,
           "RunItem::sendMessage raw data is %s", raw_readable);
  delete[] raw_readable;
  debugLog(NGB_RUN_ITEM, "RunItem::sendMessage exit...");
  return true;
}

bool RunItem::receiveMessage()
{
  debugLog(NGB_RUN_ITEM, "RunItem::receiveMessage enter...");
  Message msg;
  msg.init();
  int len = msg.parseRawToApp((char*)g_buf);
  msg.print();
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
