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

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "container.hh"
#include "log.hh"
#include "dictionary_manager.hh"
#include "value_parser.hh"
#include "avp_type.hh"
#include "parser_type.hh"
#include "utils.hh"
#include "ngb_defs.hh"

extern DictionaryManager *dictMgr;

static ValueParser* createValueParser(NGB_DATA_TYPE type)
{
  switch (type) {
  case NGB_OCTECT_STRING_TYPE:
    debugLog(NGB_CONTAINER,
             "AvpEntry::parseAppToRaw create Octect parser");
    return new OctectValueParser;
  case NGB_UTF8_STRING_TYPE:
    debugLog(NGB_CONTAINER,
             "AvpEntry::parseAppToRaw create utf8 parser");
    return new Utf8ValueParser;
  case NGB_IP_ADDRESS_TYPE:
    debugLog(NGB_CONTAINER,
             "AvpEntry::parseAppToRaw create IPaddress parser");
    return new IPaddressValueParser;
  case NGB_INTEGER32_TYPE:
    debugLog(NGB_CONTAINER,
             "AvpEntry::parseAppToRaw create integer32 parser");
    return new Integer32ValueParser;
  case NGB_INTEGER64_TYPE:
    debugLog(NGB_CONTAINER,
             "AvpEntry::parseAppToRaw create integer64 parser");
    return new Integer64ValueParser;
  case NGB_SHORT_TYPE:
    debugLog(NGB_CONTAINER,
             "AvpEntry::parseAppToRaw create short parser");
    return new ShortValueParser;
  case NGB_BYTE_TYPE:
    debugLog(NGB_CONTAINER,
             "AvpEntry::parseAppToRaw create byte parser");
    return new ByteValueParser;
  default:
    assert(0);
  }
}

bool AvpEntry::parseAppToRaw(char *raw, int &len)
{
  debugLog(NGB_CONTAINER,
           "AvpEntry::parseAppToRaw name(%s), length(%d), type(%s)",
           getName().c_str(), getLength(), getType().c_str());
  AvpType *avpt;
  AvpTypeList al;
  if ((avpt = al.search(getType().c_str())) == NULL) {
    debugLog(NGB_CONTAINER, "AvpEntry::parseAppToRaw unkown avp type");
    return false;;
  }
  ValueParser *vp = createValueParser(avpt->getType());
  // no consideration for quantity==0, since it already converted
  // to 1 in its setQuantity() method.
  // TODO: support multiple quantites, this require that parsing
  // provisioned to seperate the value to satisfy quantity requirement
  int i = 0;
  int size = 0;
  len = 0;
  for (; i < getQuantity(); i++) {
    // TODO; the getValue should be enhanced to return one value for
    // one quantity, some format of value should be designed
    vp->setValue(getValue().c_str(), VALUE_IS_TEXT);
    vp->setLength(getLength());
    vp->parseAppToRaw((void *)raw, size);
    len += size;
    // char *raw_readable = new char[size * 2 + 1];
    // int realLen = Utils::binaryToAscii(raw, size, raw_readable);
    // raw_readable[size*2] = '\0';
    // debugLog(NGB_RUN_ITEM,
    //          "RunItem::sendMessage raw data(%d/%d) is %s",
    //          realLen, size, raw_readable);
    // delete[] raw_readable;
  }
  
  delete vp;
  return true;
}

bool AvpEntry::parseRawToApp(char *raw, int &len)
{
  debugLog(NGB_CONTAINER, "AvpEntry::parseRawToApp enter...");
  AvpType *avpt;
  AvpTypeList al;
  char finalValue[1024] = {0};
  char valueOnce[128] = {0};
  if ((avpt = al.search(getType().c_str())) == NULL) {
    debugLog(NGB_CONTAINER, "AvpEntry::parseRawToApp unkown avp type");
    return false;;
  }
  ValueParser *vp = createValueParser(avpt->getType());
  int size = 0, i = 0;
  len = 0;
  for (; i < getQuantity(); i++) {
    vp->setValue(raw + size, VALUE_IS_RAW);
    vp->setLength(getLength());
    vp->parseRawToApp(valueOnce, size);
    len += size;
    strcat(finalValue, valueOnce);
  }
  setValue(std::string(finalValue));
  debugLog(NGB_CONTAINER, "AvpEntry::parseRawToApp get value %s for avp %s",
           getValue().c_str(), getName().c_str());
  delete vp;
  return true;
}

std::string AvpEntry::toString(int numOfIndent)
{
  std::string myString = "";
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  myString = myIndent + type_ + std::string("::") + avpName_ +
    std::string("(") + Utils::intToString(length_) + std::string(":") +
    Utils::intToString(quantity_) + std::string(") =: ") + value_;
  return myString;
}

std::string AvpEntry::getParentSignature()
{
  int pos = signature_.find_last_of("::");
  if (pos == -1) {
    return std::string("");
  }
  return signature_.substr(0, pos-1);
}

AvpEntry* EntryContainer::appendAvp(std::string name, std::string value)
{
  AvpEntry *avp_r = new AvpEntry;
  avp_r->setName(name);
  avp_r->setValue(value);
  avpList_.push_back(avp_r);
  return avp_r;
}

AvpEntry* EntryContainer::appendAvp(std::string name, std::string type,
                               int len, int quantity)
{
  AvpEntry *avp_r = new AvpEntry;
  avp_r->setName(name);
  avp_r->setType(type);
  avp_r->setLength(len);
  avp_r->setQuantity(quantity);
  avpList_.push_back(avp_r);
  return avp_r;
}

EntryContainer* EntryContainer::appendContainer(std::string name)
{
  GavpEntry *gavp = new GavpEntry;
  gavp->setName(name);
  avpList_.push_back((AvpEntry*)gavp);
  return gavp;
}

bool EntryContainer::addAvp(std::string name, std::string value)
{
  return true;
}

bool EntryContainer::addAvp(std::string name, std::string type, int len,
                       int quantity)
{
  return true;
}

std::string EntryContainer::toString(int numOfIndent)
{
  std::string myString = "";
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  for (std::vector<AvpEntry*>::iterator it = avpList_.begin();
       it != avpList_.end(); it++) {
    myString += (*it)->toString(numOfIndent) + std::string("\n");
  }
  return myString;
}

bool GavpEntry::parseAppToRaw(char *raw, int &len)
{
  return true;
}

bool GavpEntry::parseRawToApp(char *raw, int &len)
{
  return true;
}

bool GavpEntry::addAvp(std::string name, std::string value)
{
  return true;
}

bool GavpEntry::addAvp(std::string name, std::string type, int len,
                       int quantity)
{
  return true;
}

std::string GavpEntry::toString(int numOfIndent)
{
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  std::string myString = myIndent + type_ + std::string("::") + avpName_ +
    std::string("(") + std::string(":") + Utils::intToString(quantity_) +
    std::string(") {\n");
  for (std::vector<AvpEntry*>::iterator it = avpList_.begin();
       it != avpList_.end(); it++) {
    myString += (*it)->toString(numOfIndent + 1) + std::string("\n");
  }
  myString += myIndent + std::string("}");
  return myString;
}

bool MessageEntryContainer::parseAppToRaw(char *raw, int &offset)
{
  debugLog(NGB_CONTAINER, "Container::parseAppToRaw enter...");
  offset = 0;
  int len = 0;
  char *pos = 0; // current position the raw data will be write to
  for (std::vector<AvpEntry*>::iterator it = avpList_.begin();
       it != avpList_.end(); ++it) {
    if ((*it)->getType() == "start_of_group" ||
        (*it)->getType() == "end_of_group") {
      debugLog(NGB_CONTAINER,
               "Container::parseAppToRaw meet group avp, continue");
      continue;
    }
    pos = raw + offset;
    if (!(*it)->parseAppToRaw(pos, len)) {
      debugLog(NGB_CONTAINER,
               "Container::parseAppToRaw avpEntry fail to parseAppToRaw");
    }
    offset += len;
  }
  return true;
}

bool MessageEntryContainer::parseRawToApp(char *raw, int &offset)
{
  debugLog(NGB_CONTAINER, "Container::parseRawToApp enter...");
  offset = 0;
  int len = 0;
  char *pos = 0; // current position the raw data will be write to
  for (std::vector<AvpEntry*>::iterator it = avpList_.begin();
       it != avpList_.end(); ++it) {
    if ((*it)->getType() == "start_of_group" ||
        (*it)->getType() == "end_of_group") {
      debugLog(NGB_CONTAINER,
               "Container::parseRawToApp meet group avp, continue");
      continue;
    }
    pos = raw + offset;
    if (!(*it)->parseRawToApp(pos, len)) {
      debugLog(NGB_CONTAINER,
               "Container::parseRawToApp avpEntry fail to parseAppToRaw");
    }
    debugLog(NGB_CONTAINER, "Container::parseRawToApp get length %d", len);
    offset += len;
  }
  return true;
}

bool
MessageEntryContainer::fillAvpsWithTypesFromDictionary(std::string cmdName)
{
  debugLog(NGB_CONTAINER,
           "Container::fillAvpsWithTypesFromDictionary enter...");
  Command command;
  if (!dictMgr || !dictMgr->getCommandByName(cmdName, &command)) {
    debugLog(NGB_CONTAINER,
             "Container::fillAvpsWithTypesFromDictionary fail to get command");
    return false;
  }
  debugLog(NGB_CONTAINER,
           "Container::fillAvpsWithTypesFromDictionary command dictionary\n%s",
           command.toString().c_str());
  
  debugLog(NGB_CONTAINER,
           "Container::fillAvpsWithTypesFromDictionary exit");
  return true;
}

bool MessageEntryContainer::createAvpsFromDictionary(std::string cmdName)
{
  debugLog(NGB_CONTAINER, "Container::createAvpsFromDictionary enter ...");
  if (!dictMgr) {
    debugLog(NGB_CONTAINER,
             "Container::createAvpsFromDictionary fail to get dictMgr");
    return false;
  }
  Command command;
  if (!dictMgr->getCommandByName(cmdName, &command)) {
    debugLog(NGB_CONTAINER,
             "Container::createAvpsFromDictionary fail to get command");
    return false;
  }
  Avp avp;
  std::string name;
  std::string type;
  int length = 0;
  int quantity = 0;
  int numOfAvps = command.getNumOfAvp();
  for (int i = 0; i < numOfAvps; i++) {
    command.getAvpByIndex(i, &avp);
    name = avp.getName();
    type = avp.getType();
    length = avp.getLength();
    quantity = avp.getQuantity();
    addAvp(name, type, length, quantity);
  }
  return true;
}

void MessageEntryContainer::print()
{
  if (!avpList_.size()) {
    debugLog(NGB_CONTAINER, "Container::print no avp to print");
    return;
  }
  int num = avpList_.size();
  char buf[100*1024] = {0};
  char line[1024] = {0};
  // set the title
  sprintf(buf, "there are %d avp items\n", num);
  for (std::vector<AvpEntry*>::iterator it = avpList_.begin();
       it != avpList_.end(); ++it) {
    sprintf(line,
            "avpName(%s), avpCode(%d), length(%d), type(%s), value(%s), "
            "quantity(%d)\n", (*it)->getName().c_str(), (*it)->getCode(),
            (*it)->getLength(), (*it)->getType().c_str(),
            (*it)->getValue().c_str(), (*it)->getQuantity());
    strcat(buf, line);
  }
  debugLog(NGB_CONTAINER, "%s", buf);
}

