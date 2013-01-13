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
#include <sstream>
#include <sys/param.h>
#include "message.hh"
#include "log.hh"
#include "dictionary_manager.hh"
#include "ngb_defs.hh"

extern DictionaryManager *dictMgr;
std::map<std::string, int> signatureCounter;
std::map<std::string, RawEntry*> signatureMapping;

int incrSignatureCounter(std::string sign)
{
  if (signatureCounter.find(sign) == signatureCounter.end()) {
    signatureCounter.insert(std::pair<std::string, int>(sign, 0));
  }
  debugLog(NGB_MESSAGE,
           "Message::incrSignatureCounter cunter for %s is %d",
           sign.c_str(), signatureCounter[sign]);
  return signatureCounter[sign]++;
}

bool setSignature(std::string sign, RawEntry *rawEntry)
{
  if (signatureMapping.find(sign) != signatureMapping.end()) {
    return false;
  }
  signatureMapping.insert(std::pair<std::string, RawEntry*>(sign, rawEntry));
  return true;
}

RawEntry* fetchEntryBySignature(std::string signature)
{
  if (signatureMapping.find(signature) != signatureMapping.end()) {
    return signatureMapping[signature];
  }
  return (RawEntry*)-1;
}

#define SEPARATOR "=:"
#define GET_NAME_VALUE_PAIR(entry, name, value)           \
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

#define GET_GROUP_AVP_NAME(entry, name)                 \
  int pos = entry.find_first_of(" ");                   \
  if (pos == std::string::npos) {                       \
    debugLog(NGB_MESSAGE, "Message::get_group_avp_name" \
             " fail to parse parameter for\n%s",        \
             entry.c_str());                            \
    return false;                                       \
  }                                                     \
  name = entry.substr(0, pos);

bool isStartOfGroupAvp(std::string entry)
{
  // make sure to trim the string before calling this function
  if (entry.find_last_of("{") == -1) {
    return false;
  }
  return true;
}
bool isEndOfGroupAvp(std::string entry)
{
  // make sure to trim the string before calling this function
  if (entry.find_last_of("}") == -1) {
    return false;
  }
  return true;
}

std::string RawEntry::generateSignature()
{
  std::string signatureString = getName();
  GroupRawEntry *p = getParent();
  if (p && !(p->getSignature().empty())) {
    signature_ = p->getSignature() + std::string("::") + getName();
  } else {
    signature_ = getName();
  }
  int counter = incrSignatureCounter(signature_);
  signature_ += Utils::intToString(counter);
  return signature_;
}

std::string RawEntryContainer::toString(int numOfIndent)
{
std::string myString = "";
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  for (std::vector<RawEntry*>::iterator ite = childEntries_.begin();
       ite != childEntries_.end(); ite++) {
    myString += (*ite)->toString(numOfIndent) + std::string("\n");
  }
  return myString;
}

bool RawEntryContainer::setSignature(std::string sign, RawEntry *rawEntry)
{
  if (signatureMapping_.find(sign) != signatureMapping_.end()) {
    return false;
  }
  signatureMapping_.insert(std::pair<std::string, RawEntry*>(sign, rawEntry));
  return true;
}

RawEntry* RawEntryContainer::fetchEntryBySignature(std::string signature)
{
  if (signatureMapping_.find(signature) != signatureMapping_.end()) {
    return signatureMapping_[signature];
  }
  return (RawEntry*)-1;

}

RawEntry* GroupRawEntry::appendEntry(std::string name, std::string value)
{
  RawEntry *e = new RawEntry;
  e->setName(name);
  e->setValue(value);
  e->setParent(this);
  e->setDeepth(getDeepth() + 1);
  childEntries_.push_back(e);
  return e;
}

GroupRawEntry* GroupRawEntry::appendContainer(std::string name)
{
  GroupRawEntry* group = new GroupRawEntry();
  group->setName(name);
  group->setParent(this);
  group->setDeepth(getDeepth() + 1);
  childEntries_.push_back(group);
  return group;
}

std::string GroupRawEntry::toString(int numOfIndent)
{
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  std::string myString = myIndent + getName() + std::string(" {\n");
  for (std::vector<RawEntry*>::iterator ite = childEntries_.begin();
       ite != childEntries_.end(); ite++) {
    myString += (*ite)->toString(numOfIndent + 1) + std::string("\n");
  }
  myString += myIndent + std::string("}");
  return myString;
}

std::string RawEntry::toString(int numOfIndent)
{
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  std::string myString = myIndent + getName() + std::string(" =: ") +
    getValue() + std::string("(") + getSignature() +
    std::string(")");
  return myString;
}

bool Message::init()
{
  debugLog(NGB_MESSAGE, "Message::init enter...");
  // if the path is not provisioned, it is intended to load from
  // dictionary, no need to read message file
  if (path_.size() && !readMsgFile()) {
    debugLog(NGB_MESSAGE, "Message::init fail to read message file");
    return false;
  }
  debugLog(NGB_MESSAGE, "Message::init exit...");
  return true;
}

bool Message::fillAvpsWithTypes()
{
  return false;
}

bool Message::generateMessageAvps()
{
  debugLog(NGB_CONTAINER, "Message::generateMessageAvps enter");
  Command command;
  if (!dictMgr || !dictMgr->getCommandByName(command_, &command)) {
    debugLog(NGB_CONTAINER,
             "Message::generateMessageAvps fail to get command");
    return false;
  }
  debugLog(NGB_CONTAINER,
           "Message::generateMessageAvps command dictionary:\n%s",
           command.toString().c_str());

  std::vector<Avp*> allAvps;
  command.goThroughAllAvps(allAvps);


  ////// test code to print out all the serialized avps //////
  std::string allAvpString = "";
  for (std::vector<Avp*>::iterator ite = allAvps.begin();
       ite != allAvps.end(); ite++) {
    allAvpString += (*ite)->toString() + std::string("\n");
  }
  debugLog(NGB_CONTAINER,
           "Message::generateMessageAvps allAvps are \n%s",
           allAvpString.c_str());
  /////////////////////// END ////////////////////////////////
  
  // TODO, don't forget to delete all the item in allAvps vector
  debugLog(NGB_MESSAGE,
           "Message::generateMessageAvps number of avps are %d",
           allAvps.size());

  debugLog(NGB_MESSAGE, "Message::generateMessageAvps rawEntry are\n%s",
           rootGroupRawEntry_.toString().c_str());

  for (std::vector<Avp*>::iterator ite = allAvps.begin();
       ite != allAvps.end(); ite++) {
    AvpEntry* e = new AvpEntry;
    e->setName((*ite)->getName());
    e->setLength((*ite)->getLength());
    e->setType((*ite)->getType());
    std::string signature = (*ite)->getSignature();
    debugLog(NGB_MESSAGE,
             "Message::generateMessageAvps signature string is %s",
             signature.c_str());
    RawEntry* r = fetchEntryBySignature(signature);
    if (r == (RawEntry*)-1) {
      debugLog(NGB_MESSAGE,
               "Message::generateMessageAvps signature %s not found",
               signature.c_str());
    }
    e->setValue(r != (RawEntry*)-1 ? r->getValue() : std::string(""));
    avpContainer_.appendAvp(e);

  }
  return true;
  //return avpContainer_.fillAvpsWithTypesFromDictionary(command_);
}

bool Message::createAvpsWithNameAndType()
{
  debugLog(NGB_CONTAINER, "Message::createAvpsWithNameAndType enter");
  Command command;
  if (!dictMgr || !dictMgr->getCommandByName(command_, &command)) {
    debugLog(NGB_CONTAINER,
             "Message::createAvpsWithNameAndType fail to get command");
    return false;
  }
  debugLog(NGB_CONTAINER,
           "Message::createAvpsWithNameAndType command dictionary:\n%s",
           command.toString().c_str());

  std::vector<Avp*> allAvps;
  command.goThroughAllAvps(allAvps);


  ////// test code to print out all the serialized avps //////
  std::string allAvpString = "";
  for (std::vector<Avp*>::iterator ite = allAvps.begin();
       ite != allAvps.end(); ite++) {
    allAvpString += (*ite)->toString() + std::string("\n");
  }
  debugLog(NGB_CONTAINER,
           "Message::createAvpsWithNameAndType allAvps are \n%s",
           allAvpString.c_str());
  /////////////////////// END ////////////////////////////////
  
  // TODO, don't forget to delete all the item in allAvps vector
  debugLog(NGB_MESSAGE,
           "Message::createAvpsWithNameAndType number of avps are %d",
           allAvps.size());

  debugLog(NGB_MESSAGE, "Message::createAvpsWithNameAndType rawEntry are\n%s",
           rootGroupRawEntry_.toString().c_str());

  for (std::vector<Avp*>::iterator ite = allAvps.begin();
       ite != allAvps.end(); ite++) {
    AvpEntry* e = new AvpEntry;
    e->setName((*ite)->getName());
    e->setLength((*ite)->getLength());
    e->setType((*ite)->getType());
    avpContainer_.appendAvp(e);
  }
  //return avpContainer_.createAvpsFromDictionary(command_);
  return true;
}

//void Message::setUdpHeader(udp_header h) {
//  strncpy((char *)&hdr_, (char *)&h, sizeof(udp_header));
//}

bool Message::parseCommand(std::string entry)
{
  debugLog(NGB_MESSAGE, "Message::parseCommand enter");
  std::string name, value;
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
  // get and set the command code from dictionary
  std::stringstream(dictMgr->getCommandCode(command_)) >> commandCode_;
  return true;
  debugLog(NGB_MESSAGE, "Message::parseCommand exit");
}

bool Message::readMsgFile()
{
  debugLog(NGB_MESSAGE, "Message::readMsgFile enter...");
  debugLog(NGB_MESSAGE,
           "Message::readMsgFile input file is %s", path_.c_str());
  char line[512];
  std::string name, value, entry;
  std::ifstream msgFile(path_.c_str(), std::ifstream::in);
  msgFile.getline(line, 512);
  // firstly get the command definition, the command should be defined
  // at the first line of message file, null value of command parameter
  // is still invalide

  entry = std::string(line);
  // while (msgFile.getline(line, 512)) {
  //     debugLog(NGB_MESSAGE, "Message::readMsgFile line is %s", line);
  // }
  if (!parseCommand(entry)) return false;
  // default to 3 levels deepth support for a message
  GroupRawEntry *containerLevels[3] = {0};
  int currentLevel = 0;
  containerLevels[0] = &rootGroupRawEntry_;
  while (msgFile.getline(line, 512)) {
    entry = std::string(line);
    entry = Utils::trim(entry);
    if (!entry.size() || Utils::isComments(entry))
      continue;
    if (isStartOfGroupAvp(entry)) {
      GET_GROUP_AVP_NAME(entry, name);
      GroupRawEntry *ec =
        containerLevels[currentLevel]->appendContainer(name);
      ec->generateSignature();
      containerLevels[++currentLevel] = ec;
    } else if (isEndOfGroupAvp(entry)) {
      if (--currentLevel < 0) {
        debugLog(NGB_MESSAGE,
                 "Message::readMsgFile, invalid group termination");
        return false;
      }
      continue;
    } else {
      GET_NAME_VALUE_PAIR(entry, name, value);
      RawEntry *e = containerLevels[currentLevel]->appendEntry(name, value);
      std::string sign = e->generateSignature();
      debugLog(NGB_MESSAGE,
               "Message::readMsgFile signature for %s is %s",
               e->getName().c_str(), sign.c_str());
      if (!setSignature(sign, e)) {
        debugLog(NGB_MESSAGE,
                 "Message::readMsgFile fail to set signature, duplicated");
        return false;
      }
    }
  }
  debugLog(NGB_MESSAGE, "Message::readMsgFile exit");
  return true;
}

void Message::printRawEntry()
{
  debugLog(NGB_MESSAGE,
           "print all the avps for command %s", command_.c_str());
  debugLog(NGB_MESSAGE,
           "%s", rootGroupRawEntry_.toString().c_str());
}

void Message::print()
{
  debugLog(NGB_MESSAGE,
           "print all the avps for command %s", command_.c_str());
  avpContainer_.print();
}

int Message::parseAppToRaw(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseAppToRaw enter...");
  // if (!fillAvpsWithTypes()) {
  //   debugLog(NGB_MESSAGE, "Message::init fail to fullfillAvps");
  //   return false;
  // }
  if (!generateMessageAvps()) {
    debugLog(NGB_MESSAGE, "Message::init fail to fullfillAvps");
    return false;
  }
  
  print();
  char *p = raw;
  int hdrLen = parseHdrAppToRaw(p);
  debugLog(NGB_MESSAGE, "Message::parseAppToRaw hdrLen is %d", hdrLen);
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
  memset(&hdr_, 0, sizeof(struct dfs_msg_header));

  char bout[40] = {0};
  Utils::binaryToAscii(raw, 10, (char*)bout);
  debugLog(NGB_MESSAGE, "Message::sendMessage app to raw \n%s", bout);
  
  memcpy(&hdr_, raw, sizeof(dfs_msg_header));
  commandCode_ = hdr_.op;
  // code check whether it is a error message
  if (hdr_.reply == 1) {
    debugLog(NGB_MESSAGE,
             "Message::parseHdrRawToApp get error message for command %d, "
             "error code is %d", commandCode_, hdr_.errcode);
  }
  std::stringstream ss;
  ss << commandCode_;
  command_ = dictMgr->getCommandName(ss.str());
  debugLog(NGB_MESSAGE,
           "Message::parseHdrRawToApp get %s message", command_.c_str());
  return sizeof(dfs_msg_header);
}

int Message::parseHdrAppToRaw(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseHdrAppToRaw enter...");
  memset(&hdr_, 0, sizeof(struct dfs_msg_header));
  hdr_.op = commandCode_;
  memcpy(raw, &hdr_, sizeof(dfs_msg_header));
  return sizeof(dfs_msg_header);
}

int Message::parseBodyRawToApp(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseBodyRawToApp enter...");
  if (!createAvpsWithNameAndType()) {
    debugLog(NGB_MESSAGE,
             "Message::parseBodyRawToApp create container avps fail");
  }
  int len = 0;
  avpContainer_.parseRawToApp(raw, len);
  if (!convertToRaw()) {
    debugLog(NGB_MESSAGE, "fail to convertToRaw");
    return -1;
  }
  return len;
}

int Message::parseBodyAppToRaw(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseBodyAppToRaw enter...");
  int len = 0;
  avpContainer_.parseAppToRaw(raw, len);
  return len;
}

bool Message::convertToRaw()
{
  GroupRawEntry *group_array[3] = {0};
  int currentLevel = 0;
  std::string lastSignature = "";
  bool isSameParent = false;
  group_array[0] = &rootGroupRawEntry_;
  for (std::vector<AvpEntry*>::iterator ite = avpContainer_.avpList_.begin();
       ite != avpContainer_.avpList_.end(); ite++) {
    if ((*ite)->getType() == "start_of_group") {
      GroupRawEntry *ec =
        group_array[currentLevel]->appendContainer((*ite)->getName());
      group_array[++currentLevel] = ec;
    } else if ((*ite)->getType() == "end_of_group") {
      if (--currentLevel < 0) {
        debugLog(NGB_MESSAGE, "Message::convertToRaw invalid group termination");
        return false;
      }
      continue;
    } else {
      RawEntry *e = group_array[currentLevel]->appendEntry((*ite)->getName(),
                                                           (*ite)->getValue());
    }
  }
  printRawEntry();
  return true;
  debugLog(NGB_MESSAGE, "Message::convertToRaw exit");
}
