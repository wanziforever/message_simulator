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
// signature is used for finding the raw avp provisioned in the message,
// the dictionary data and raw data will both generate the signature data,
// and signature is the bridge between dictionary and raw data; signature
// data is generated by the following way, we call it the obsoluted path
// to the specified tpye of data:
// grandParentAVPName::parentAVPName::myAVPName
// if there is more than one instance for this type, (like an array):
// grandParentAVPName::parentAVPName::myAVPName0
// grandParentAVPName::parentAVPName::myAVPName1
// grandParentAVPName::parentAVPName::myAVPName2
// or
// grandParentAVPName::parentAVPName0::myAVPName0
// grandParentAVPName::parentAVPName1::myAVPName0
// grandParentAVPName::parentAVPName2::myAVPName0
// 
// the message will be setuped in the order defined in the dictionary file,
// (the order in the raw message fill will take no means), tool will firstly
// read the the command name provisioned in the raw message file, and than
// search its definition in the dictionary, get all its defined AVPs in order,
// (including the normal AVP, and group AVP), store them to some structure,
// than tool will serialize all the all the AVPs to a "one level" structure in
// order, generate signatures for each serialized AVP item, and get related
// values through the signature for each AVP item, put all AVPs to the container
// that going to be send to network.
// Author: wangliang8@hisense.com (Denny Wang)
// History: created @2012/10/10

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <sys/param.h>
#include <algorithm>
#include "message.hh"
#include "log.hh"
#include "ngb_defs.hh"

extern DictionaryManager *dictMgr;
std::map<std::string, int> signatureCounter;
std::map<std::string, RawEntry*> signatureMapping;

// the command field in message file should have the following format
// command =: XXX_XXX_XXX(R) or command =: YYY_YYY_YYY(A)
bool parseDirectionMode(const std::string value,
                        std::string &command,
                        short &direction)
{
  int spos = value.find_first_of("(");
  int epos = value.find_first_of(")");
  if (spos == value.npos || epos == value.npos || epos < spos+1) {
    debugLog(NGB_MESSAGE,
             "cannot find direction string, use request defualt");
    direction = REQUEST;
    command = value;
    return true;
  }
  command = value.substr(0, spos);
  std::string directionString = value.substr(spos, spos + 1);
  debugLog(NGB_MESSAGE,
           "direction string is %s", directionString.c_str());
  transform(directionString.begin(),
            directionString.end(),
            directionString.begin(),
            ::toupper);
  direction = (directionString == "A" ? ANSWER : REQUEST);
  return true;
}
// increase the registered signature counter, which is used for more than
// than one quantities of avps, the counter is used to generate the signature
// for multiple avp instance for one type
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
  signatureMapping.insert(std::pair<std::string,
                          RawEntry*>(sign, rawEntry));
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
    debugLog(NGB_ERROR, "Message::GET_NAME_VALUE_PAIR"  \
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
    debugLog(NGB_ERROR, "Message::get_group_avp_name"   \
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
  int newNumOfIndent = numOfIndent;
  std::string myString = "";
  if (!isRoot()) {
    myString = myIndent + getName() + std::string(" {\n");
    newNumOfIndent = numOfIndent + 1;
  }
  for (std::vector<RawEntry*>::iterator ite = childEntries_.begin();
       ite != childEntries_.end(); ite++) {
    myString += (*ite)->toString(newNumOfIndent) + std::string("\n");
  }
  if (!isRoot()) {
    myString += myIndent + std::string("}");
  }
  return myString;
}

std::string GroupRawEntry::getDisplayData()
{
  // currently simply use the toString() as the displayed data, although
  // toString is designed for debug use
  std::string display = toString();
  return display;
}

std::string RawEntry::toString(int numOfIndent)
{
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  std::string myString = myIndent + getName() + std::string(" =: ") +
    getValue()/* + std::string("(") + getSignature() +
                   std::string(")")*/;
  return myString;
}

std::string RawEntry::getDisplayData()
{
  // currently simply use toString as the displayed data, although the
  // toString is designed for debug log
  std::string display = toString();
  return display;
}

bool Message::init()
{
  debugLog(NGB_MESSAGE, "Message::init enter...");
  // if the path is not provisioned, it is intended to load from
  // dictionary, no need to read message file
  if (path_.size() && !readMsgFile()) {
    debugLog(NGB_ERROR, "Message::init fail to read message file");
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
  if (!dictMgr ||
      !dictMgr->getCommandByName(command_, &command, direction_)) {
    debugLog(NGB_ERROR,
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
      debugLog(NGB_ERROR,
               "Message::generateMessageAvps signature %s not found, "
               "used default", signature.c_str());
      //return false;
    }
    e->setSignature(signature);
    if (r != (RawEntry*)-1) {
      r->setAvpEntryLink(e); // for performance implementation
    }
    e->setValue(r != (RawEntry*)-1 ? r->getValue() : std::string(""));
    avpContainer_.appendAvp(e);
  }
  return true;
  //return avpContainer_.fillAvpsWithTypesFromDictionary(command_);
}

void Message::updateEntryBySignature(std::string signature, std::string value)
{
  if (signatureMapping.find(signature) == signatureMapping.end()) {
    debugLog(NGB_CONTAINER, "Message::updatePerformanceData "
             "fail to find the signature %s", signature.c_str());
    return;
  }

  RawEntry *r = signatureMapping[signature];
  AvpEntry* e = r->getAvpEntryLink();
  e->setValue(value);
  char *raw = e->getDataPointer();
  int len = 0;
  e->parseAppToRaw(raw, len);
}

bool Message::createAvpsWithNameAndType()
{
  debugLog(NGB_CONTAINER, "Message::createAvpsWithNameAndType enter");
  Command command;
  if (!dictMgr ||
      !dictMgr->getCommandByName(command_, &command, direction_)) {
    debugLog(NGB_ERROR,
             "Message::createAvpsWithNameAndType fail to get command");
    return false;
  }
  debugLog(NGB_CONTAINER,
           "Message::createAvpsWithNameAndType command dictionary:\n%s",
           command.toString().c_str());

  std::vector<Avp*> allAvps;
  command.goThroughAllAvps(allAvps);

  //////// test code to print all the serialized avps /////////
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
  bodySize_ = 0; // since the function may called twice, reset to 0
  for (std::vector<Avp*>::iterator ite = allAvps.begin();
       ite != allAvps.end(); ite++) {
    AvpEntry* e = new AvpEntry;
    e->setName((*ite)->getName());
    e->setLength((*ite)->getLength());
    bodySize_ += (*ite)->getLength();
    e->setType((*ite)->getType());
    avpContainer_.appendAvp(e);
  }
  //return avpContainer_.createAvpsFromDictionary(command_);
  return true;
}

bool Message::parseCommand(std::string entry)
{
  debugLog(NGB_MESSAGE, "Message::parseCommand enter");
  std::string name, value;
  GET_NAME_VALUE_PAIR(entry, name, value);
  if (name != "command") {
    debugLog(NGB_ERROR,
             "Message::readMsgFile cannot get command from message file\n"
             "the command definition should be the first line.");
    return false;
  }
  if (value.empty()) {
    debugLog(NGB_ERROR,
             "Message::readMsgFile the command field cannot be null");
    return false;
  }
  // get the message mode(direction), request or answer
  // the mode char follow the command name as COMMAND_NAME(R) or
  // COMMAND_NAME(A)
  parseDirectionMode(value, command_, direction_);
  // get and set the command code from dictionary
  std::stringstream(dictMgr->getCommandCode(command_)) >> commandCode_;
  return true;
  debugLog(NGB_MESSAGE, "Message::parseCommand exit");
}

bool Message::readMsgFile()
{
  debugLog(NGB_MESSAGE, "Message::readMsgFile enter...");
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
        debugLog(NGB_ERROR,
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
        debugLog(NGB_ERROR,
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

std::string Message::getDisplayData()
{
  std::string display = "";
  if (errorCode_ == 1) {
    display += "error message received!!!\n";
  }
  display += std::string("COMMAND NAME IS: ") + command_ + std::string("\n");
  display += rootGroupRawEntry_.getDisplayData();
  return display;
}

void Message::printDebug()
{
  debugLog(NGB_MESSAGE,
           "print all debug avps for command %s", command_.c_str());
  avpContainer_.print();
}

int Message::parseAppToRaw(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseAppToRaw enter...");

  if (parseAppToRawDone_) {
    return 0;
  }

  //while (1) {
  //  debugLog(NGB_MESSAGE, "Message::parseAppToRaw enter...");
  //}

  if (!generateMessageAvps()) {
    debugLog(NGB_ERROR, "Message::init fail to fullfillAvps");
    return -1;
  }
  
  char *p = raw;
  int hdrLen = parseHdrAppToRaw(p);
  debugLog(NGB_MESSAGE, "Message::parseAppToRaw hdrLen is %d", hdrLen);
  p = p + hdrLen;
  int bodyLen = parseBodyAppToRaw(p);
  parseAppToRawDone_ = true;
  return hdrLen + bodyLen;
}

int Message::parseRawToApp(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseRawToApp enter...");
  char *p = raw;
  int hdrLen = parseHdrRawToApp(p);
  if (hdrLen == -1) {
    debugLog(NGB_MESSAGE, "Message::parseRawToApp fail to parse header");
    return -1;
  }
  p = p + hdrLen;
  int bodyLen = parseBodyRawToApp(p);
  if (bodyLen == -1) {
    debugLog(NGB_MESSAGE, "Message::parseRawToApp fail to parse body");
    return -1;
  }
  return hdrLen + bodyLen;
}

int Message::parseHdrRawToApp(char *raw)
{
  debugLog(NGB_MESSAGE, "Message::parseHdrRawToApp enter...");
  memset(&hdr_, 0, sizeof(struct dfs_msg_header));

  memcpy(&hdr_, raw, sizeof(dfs_msg_header));
  commandCode_ = hdr_.op;
  // code check whether it is a error message
  if (hdr_.reply == 1) {
    debugLog(NGB_MESSAGE,
             "Message::parseHdrRawToApp get error message for command %d, "
             "error code is %d", commandCode_, hdr_.errcode);
    errorCode_ = hdr_.reply;
  }
  debugLog(NGB_MESSAGE,
             "Message::parseHdrRawToApp get comamnd code %d from message",
             commandCode_);
  std::stringstream ss;
  ss << commandCode_;
  
  command_ = dictMgr->getCommandName(ss.str());
  dictMgr->getCommandName(ss.str());

  if (!command_.size()) {
    debugLog(NGB_MESSAGE,
             "Message::parseHdrRawToApp get unsupport comamnd code %d",
             commandCode_);
    return -1;
  }
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
    debugLog(NGB_ERROR,
             "Message::parseBodyRawToApp create container avps fail");
  }
  int len = 0;
  avpContainer_.parseRawToApp(raw, len);
  if (!convertToRaw()) {
    debugLog(NGB_ERROR, "fail to convertToRaw");
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
        debugLog(NGB_ERROR,
                 "Message::convertToRaw invalid group termination");
        return false;
      }
      continue;
    } else {
      RawEntry *e = group_array[currentLevel]->appendEntry((*ite)->getName(),
                                                           (*ite)->getValue());
    }
  }
  printRawEntry();
  debugLog(NGB_MESSAGE, "Message::convertToRaw exit");
  return true;
}
