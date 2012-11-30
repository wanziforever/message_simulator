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
#include "message.hh"
#include "log.hh"

#define SEPARATOR "=:"
#define GET_NAME_VALUE_PAIR(entry, name, value) \
  int pos = entry.find_first_of(SEPARATOR);               \
  if (pos == std::string::npos) {                         \
    debugLog(NGB_MESSAGE, "Message::GET_NAME_VALUE_PAIR"  \
             " fail to parse parameter for\n%s",          \
             entry.c_str());                              \
      return false;                                       \
  }                                                       \
  name = entry.substr(0, pos);                            \
  value = entry.substr(pos+2/*, toTheEnd*/);              \
  name = Utils::trim(name);                               \
  value = Utils::trim(value);

bool Message::init()
{
  debugLog(NGB_MESSAGE, "Message::init enter...");
  // if the path is not provisioned, no need to read file
  if (path_.size() && !readMsgFile()) {
    debugLog(NGB_MESSAGE, "Message::init fail to read message file");
    return false;
  }
  debugLog(NGB_MESSAGE, "Message::init exit...");
  return true;
}

bool Message::fillAvpsWithTypes()
{
  return avpContainer_.fillAvpsWithTypesFromDictionary(command_);
}

bool Message::createAvpsWithNameAndType()
{
  command_ = "DFS_INPUT_CHUNK";
  return avpContainer_.createAvpsFromDictionary(command_);
}

bool Message::readMsgFile()
{
  debugLog(NGB_MESSAGE, "Message::readMsgFile enter...");
  char line[512];
  std::string name;
  std::string value;
  std::ifstream msgFile(path_.c_str(), std::ifstream::in);
  std::string entry;
  // firstly get the command definition, the command should be defined
  // at the first line of message file, null value of command parameter
  // is still invalide
  msgFile.getline(line, 512);
  entry = std::string(line);
  GET_NAME_VALUE_PAIR(entry, name, value);
  if (name != "command") {
    debugLog(NGB_MESSAGE,
             "Message::readMsgFile cannot get command from message file\n"
             "the command definition should be the first line.");
    return false;
  }
  if (value.empty()) {
    debugLog(NGB_MESSAGE,
             "Message::readMsgFile the command field cannot be null");
    return false;
  }
  command_ = value;
  // read the avps for each line, and add it to request container
  while (msgFile.getline(line, 512)) {
    entry = std::string(line);
    entry = Utils::trim(entry);
    if (!entry.size() || Utils::isComments(entry))
      continue;
    GET_NAME_VALUE_PAIR(entry, name, value);
    avpContainer_.addAvp(name, value);
  }
  debugLog(NGB_MESSAGE, "Message::readMsgFile exit...");
  return true;
}

void Message::print()
{
  debugLog(NGB_MESSAGE,
           "print all the avps for command %s", command_.c_str());
  debugLog(NGB_MESSAGE, "avps in message is:");
  avpContainer_.print();
}

int Message::parseAppToRaw(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseAppToRaw enter...");
  if (!fillAvpsWithTypes()) {
    debugLog(NGB_MESSAGE, "Message::init fail to fullfillAvps");
    return false;
  }
  char *p = raw;
  int hdrLen = parseHdrAppToRaw(p);
  p = p + hdrLen;
  int bodyLen = parseBodyAppToRaw(p);

  return hdrLen + bodyLen;
}

int Message::parseRawToApp(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseRawToApp enter...");
  char *p = raw;
  int hdrLen = parseHdrRawToApp(p);
  p = p + hdrLen;
  int bodyLen = parseBodyRawToApp(p);
  return hdrLen + bodyLen;
}

int Message::parseHdrRawToApp(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseHdrRawToApp enter...");
  return 0;
}

int Message::parseHdrAppToRaw(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseHdrAppToRaw enter...");
  return 0;
}

int Message::parseBodyRawToApp(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseBodyRawToApp enter...");
  if (!createAvpsWithNameAndType()) {
    debugLog(NGB_MESSAGE,
             "Message::parseBodyRawToApp create container avps fail");
  }
  int len = avpContainer_.parseRawToApp(raw);
  return 0;
}

int Message::parseBodyAppToRaw(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseBodyAppToRaw enter...");
  int len = avpContainer_.parseAppToRaw(raw);
  return len;
}

