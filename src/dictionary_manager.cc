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

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include "dictionary_manager.hh"
#include "DOMTreeErrorReporter.hh"
#include "log.hh"
#include "utils.hh"
#include "ngb_defs.hh"

using namespace xercesc;
// when want to parse a tag, add a XMLCH type string here for convinent
// following XMLCH will not released forever, define just once, all the
// instance was initialized in the DictionaryManager::init() function
static const XMLCh *g_xmlch_command = NULL;
static const XMLCh *g_xmlch_avp = NULL;
static const XMLCh *g_xmlch_avpRule = NULL;
static const XMLCh *g_xmlch_grouped = NULL;
static const XMLCh *g_xmlch_attribute_name = NULL;
static const XMLCh *g_xmlch_attribute_code = NULL;
static const XMLCh *g_xmlch_attribute_type = NULL;
static const XMLCh *g_xmlch_attribute_length = NULL;
static const XMLCh *g_xmlch_attribute_quantity = NULL;

// should be initialized after XMLPlatform initialized
#define INITIALIZE_XMLCH_TAG                                      \
  g_xmlch_command = XMLString::transcode("command");              \
  g_xmlch_avp = XMLString::transcode("avp");                      \
  g_xmlch_avpRule = XMLString::transcode("avprule");              \
  g_xmlch_grouped = XMLString::transcode("grouped");              \
  g_xmlch_attribute_name = XMLString::transcode("name");          \
  g_xmlch_attribute_code = XMLString::transcode("code");          \
  g_xmlch_attribute_type = XMLString::transcode("type");          \
  g_xmlch_attribute_length = XMLString::transcode("length");      \
  g_xmlch_attribute_quantity = XMLString::transcode("quantity");

static XercesDOMParser::ValSchemes gValScheme = XercesDOMParser::Val_Auto;

bool Command::addAvp(Avp *avp)
{
  if (numOfAvp_ == size_) //avp array is full
    return false;
  avps_[numOfAvp_] = avp;
  numOfAvp_++;
  return true;
}

bool Command::getAvpByIndex(int index, Avp *output) {
  assert(output);
  if(index<0 || index>=numOfAvp_)
    return false;
  output = avps_[index];
  return true;
}

bool Command::hasAvp(std::string &findName)
{
  int index = 0;
  for(; index<numOfAvp_; ++index) {
    if(avps_[index]->getName() == findName) {
      return true;
    }
  }
  return false; // not found match
}

Avp* Command::findAvp(std::string &findName)
{
  int index = 0;
  for (; index < numOfAvp_; ++index) {
    if (avps_[index]->getType() == std::string("grouped")) {
      return ((GroupAvp*)(avps_[index]))->findAvp(findName);
    }
    if (avps_[index]->getName() == findName) {
      return avps_[index];
    }
  }
  return (Avp*)-1;
}

std::string Command::toString(int numOfIndent)
{
  debugLog(NGB_DICT_MGR, "Command::toString enter");
  int index = 0;
  std::string myString = "";
  for (; index < numOfAvp_; ++index) {
    myString += avps_[index]->toString(numOfIndent) + std::string("\n");
  }
  debugLog(NGB_DICT_MGR, "Command::toString exit");
  return myString;
}

bool Command::goThroughAllAvps(std::vector<Avp*> &allAvps)
{
  //debugLog(NGB_DICT_MGR, "Command::goThroughAllAvps enter");
  for (int i = 0; i< numOfAvp_; i++) {
    avps_[i]->goThroughSelf(allAvps);
    avps_[i]->setParent(0);
  }
  //debugLog(NGB_DICT_MGR, "Command::goThroughAllAvps exit");
  return true;
}

bool Avp::goThroughSelf(std::vector<Avp*> &allAvps)
{
debugLog(NGB_DICT_MGR,
         "Avp::goThroughSelf for %s enter, quantity = %d",
           getName().c_str(), getQuantity());
  for (int i = 0; i < (getQuantity() == 0 ? 1 : getQuantity()); i++) {
    Avp *avp = new Avp((*this));
    avp->setName(avp->getName() + Utils::intToString(i));
    avp->genSignature();
    debugLog(NGB_DICT_MGR,
             "Avp::goThroughSelf signature for Avp %s is %s",
             avp->getName().c_str(), avp->getSignature().c_str());
    allAvps.push_back(avp);
  }
  debugLog(NGB_DICT_MGR, "Avp::goThroughSelf exit");
  return true;
}

void Avp::setParent(GroupAvp* pGroupAvp) {
  parent_ = pGroupAvp;
  if (pGroupAvp != (GroupAvp*)0) {
    deepth_ = pGroupAvp->getDeepth() + 1;
  }
}

std::string Avp::genSignature()
{
  debugLog(NGB_DICT_MGR,
           "Avp::genSignature enter for AVP %s", getName().c_str());
  GroupAvp* p = getParent();
  if (p) {
    signature_ = p->getSignature() + std::string("::") + getName();
  } else {
    signature_ = getName();
  }
  debugLog(NGB_DICT_MGR,
           "Avp::genSignature exit with signature %s",
           signature_.c_str());
  return signature_;
}

bool GroupAvp::goThroughSelf(std::vector<Avp*> &allAvps)
{
  //debugLog(NGB_DICT_MGR, "GroupAvp::goThroughSelf enter");
  for (int i = 0; i < (getQuantity() == 0 ? 1 : getQuantity()); i++) {
    signature_ = genSignature() + Utils::intToString(i);
    Avp *startGroup = new GroupAvp;
    startGroup->setName(getName());
    startGroup->setType("start_of_group");
    allAvps.push_back(startGroup);
    for (std::vector<Avp*>::iterator ite = subAvpList_.begin() ;
         ite != subAvpList_.end(); ite++) {
      (*ite)->setParent(this);
      (*ite)->goThroughSelf(allAvps);
    }
    Avp *endGroup = new GroupAvp;
    startGroup->setName(getName());
    endGroup->setType("end_of_group");
    allAvps.push_back(endGroup);
  }
  //debugLog(NGB_DICT_MGR, "GroupAvp::goThroughSelf exit");
  return true;
}

// get next sibling ELEMENT_NODE type tag, usually, the getNextSibling
// function call will return the any type node, but normally, we only
// take care the ELEMENT_NODE type tag.
DOMNode* getNextElementNode(DOMNode *cur)
{
  DOMNode *next = cur->getNextSibling();
  for (; next != NULL; next=next->getNextSibling()) {
    if (next->getNodeType() == DOMNode::ELEMENT_NODE) {
      return next;
    }
  }
  return next;
}
// get attribute by attribute name for give DOM NODE
std::string getAttributeValue(const DOMNode *n, const XMLCh *name)
{
  DOMNode *attrNode = NULL;
  if ((attrNode = n->getAttributes()->getNamedItem(name)) == NULL) {
    return std::string("");
  }
  const XMLCh *value = attrNode->getNodeValue();
  return std::string(XMLString::transcode(value));
}

// get avp type for a given avp node
std::string getAvpType(DOMNode *n)
{
  DOMNode *m = n->getFirstChild();
  m = getNextElementNode(m);
  if (XMLString::equals(m->getNodeName(), g_xmlch_attribute_type)) {
    return getAttributeValue(m, g_xmlch_attribute_name);
  } else if (XMLString::equals(m->getNodeName(),g_xmlch_grouped)) {
    return std::string("grouped");
  } else {
    return NULL;
  }
}

DictionaryManager::DictionaryManager()
{
  parser_= NULL;
  errorReporter_= NULL;
}

DictionaryManager::~DictionaryManager()
{
  if(errorReporter_) {
    delete errorReporter_;
  }
  if(parser_) {
    delete parser_;
  }
  XMLPlatformUtils::Terminate();
}



bool DictionaryManager::init(const std::string &dictFile)
{
  debugLog(NGB_DICT_MGR, "DcitionaryManager::init enter...");
  // return if the parser is already inited
  if(parser_) {
    return true;
  }
  // initialize the XML4C2 system
  try {
    XMLPlatformUtils::Initialize();
  }
  catch(const XMLException &toCatch) {
    std::cerr << "Error during Xerces-c Initialization.\n"
              << " Exception message:"
              << StrX(toCatch.getMessage()) << std::endl;
    return false;
  }
  INITIALIZE_XMLCH_TAG;
  // Create our parser, then attach an error handler to the parser.
  // The parser will call back to methods of the ErrorHandler if it
  // discovers errors during the course of parsing the XML docuemnt.
  parser_ = new XercesDOMParser;
  errorReporter_ = new DOMTreeErrorReporter();
  parser_->setErrorHandler(errorReporter_);
  parser_->setValidationScheme(gValScheme);

  try {
    parser_->parse(dictFile.c_str());
  }
  catch(...) {
    debugLog(NGB_ERROR, "DOM error occured\n");
  }
}

int DictionaryManager::getNumOfTagByName(const XMLCh *name)
{
  DOMDocument *doc = parser_->getDocument();
  return doc->getElementsByTagName(name)->getLength();
}

int DictionaryManager::getNumOfCommand()
{
  return getNumOfTagByName(g_xmlch_command);
}

int DictionaryManager::getNumOfAvp()
{
  return getNumOfTagByName(g_xmlch_avp);
}

bool DictionaryManager::getAllCommands(Command *output)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::getAllCommands enter...");
  assert(output);
  // some DUPLICATED CODE, consider to optimize it
  DOMDocument *doc = parser_->getDocument();
  DOMNodeList *avpNodeList = doc->getElementsByTagName(g_xmlch_command);
  Command *commandPtr = output;
  for (int i = 0; i < avpNodeList->getLength(); i++) {
    if (!getOneCommand(avpNodeList->item(i), commandPtr)) {
      return false;
    }
    commandPtr += sizeof(Command);
  }
  return true;
}

std::string DictionaryManager::getCommandCode(std::string cmdName)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::getCommandCode enter");
  DOMDocument *doc = parser_->getDocument();
  DOMNodeList *avpNodeList = doc->getElementsByTagName(g_xmlch_command);
  std::string eachName;
  DOMNode *n = 0;
  for (int i = 0; i < avpNodeList->getLength(); i++) {
    n = avpNodeList->item(i);
    eachName = getAttributeValue(n, g_xmlch_attribute_name);
    if (eachName == cmdName) {
      return getAttributeValue(n, g_xmlch_attribute_code);
    }
  }
  debugLog(NGB_DICT_MGR,
           "DictionaryManager::getCommandCode not found command(%s)",
           cmdName.c_str());
  // TODO: how to design return value to for the not found case
  return std::string("0"); 
}

std::string DictionaryManager::getCommandName(std::string cmdCode)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::getCommandName enter");
  DOMDocument *doc = parser_->getDocument();
  DOMNodeList *avpNodeList = doc->getElementsByTagName(g_xmlch_command);
  std::string eachCode;
  DOMNode *n = 0;
  for (int i = 0; i < avpNodeList->getLength(); i++) {
    n = avpNodeList->item(i);
    eachCode = getAttributeValue(n, g_xmlch_attribute_code);
    if (eachCode == cmdCode) {
      return getAttributeValue(n, g_xmlch_attribute_name);
    }
  }
  debugLog(NGB_DICT_MGR,
           "DictionaryManager::getCommandName not found command code(%s)",
           cmdCode.c_str());
  // TODO: how to design return value to for the not found case
  return std::string(""); 
}

bool DictionaryManager::getOneCommand(const DOMNode *n, Command *output)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::getOneCommand enter");
  assert(n && output);
  Command *command = output;
  command->setName(getAttributeValue(n, g_xmlch_attribute_name));
  command->setCode(getAttributeValue(n, g_xmlch_attribute_code));
  // get the avprule sub tag for avps contained in this command
  // add the avp to the command instance
  DOMNode *m = n->getFirstChild();
  for (m = getNextElementNode(m); m != NULL; m = getNextElementNode(m)) {
    if (!XMLString::equals(m->getNodeName(), g_xmlch_avpRule)) {
      continue;
    }
    Avp *avp = getAvpByName(getAttributeValue(m, g_xmlch_attribute_name));
    if (avp == (Avp*)-1) {
      debugLog(NGB_DICT_MGR,
               "DictionaryManager::getOneCommand fail to get avp by Name");
      return false;
    }
    avp->setQuantity(getAttributeValue(m, g_xmlch_attribute_quantity));
    command->addAvp(avp);
  }
  debugLog(NGB_DICT_MGR, "DictionaryManager::getOneCommand exit");
  return true;
}

bool DictionaryManager::getCommandByName(std::string name, Command *output)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::getCommandByName enter...");
  assert(output);
  // TODO: some DUPLICATED CODE, consider to optimize it
  DOMDocument *doc = parser_->getDocument();
  DOMNodeList *avpNodeList = doc->getElementsByTagName(g_xmlch_command);
  Command *commandPtr = output;
  std::string eachName;
  debugLog(NGB_DICT_MGR,
           "DictionaryManager::getCommandByName input name is %s",
           name.c_str());
  for (int i = 0; i < avpNodeList->getLength(); i++) {
    eachName = getAttributeValue(avpNodeList->item(i), g_xmlch_attribute_name);
    if ( eachName == name ) {
      return getOneCommand(avpNodeList->item(i), commandPtr);
    }
  }
  debugLog(NGB_DICT_MGR,
           "DictionaryManager::getCommandByName fail to command by name");
  return false;
}

Avp* DictionaryManager::getAvpByName(const std::string &name)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::getAvpByName enter...");
  DOMDocument *doc = parser_->getDocument();
  DOMNodeList *avpNodeList = doc->getElementsByTagName(g_xmlch_avp);
  bool found = false;
  DOMNode *n = 0;
  for (int i = 0; i < avpNodeList->getLength(); i++) {
    n = avpNodeList->item(i);
    if (getAttributeValue(n, g_xmlch_attribute_name) == name) {
      found = true;
      break;
    }
  }
  if (found == false) {
    debugLog(NGB_DICT_MGR,
             "DictionaryManager::getAvpByName cannot get \"%s\"",
             name.c_str());
    return (Avp *)-1;
  }

  if (getAvpType(n) == "grouped") {
    return parseGroupAvp(n);
  } else {
    return parseAvp(n);
  }
  debugLog(NGB_DICT_MGR, "DictionaryManager::getAvpByName error");
  return (Avp *)-1;
}

#define SET_AVP_COMMON_ATTRIBUTE(avp, n)                                \
  avp->setName(getAttributeValue(n, g_xmlch_attribute_name));           \
  avp->setCode(getAttributeValue(n, g_xmlch_attribute_code));           \
  avp->setLength(getAttributeValue(n, g_xmlch_attribute_length));       \
  avp->setType(getAvpType(n));/*avptype is a sub tag of the avp node*/

Avp* DictionaryManager::parseAvp(DOMNode *n)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::parseAvp %s enter",
           getAttributeValue(n, g_xmlch_attribute_name).c_str());
  Avp *avp = new Avp;
  SET_AVP_COMMON_ATTRIBUTE(avp, n);
  debugLog(NGB_DICT_MGR, "DictionaryManager::parseAvp exit");
  return avp;
}

GroupAvp* DictionaryManager::parseGroupAvp(DOMNode *n)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::parseGroupAvp %s enter",
           getAttributeValue(n, g_xmlch_attribute_name).c_str());
  GroupAvp *gavp = new GroupAvp;
  SET_AVP_COMMON_ATTRIBUTE(gavp, n);
  // use twice getFirstChild before getting avprule for group avp, sincce
  // first one is to get grouped tag, and second is to get start with
  // avprule
  DOMNode *m = getNextElementNode(n->getFirstChild());
  for (m = getNextElementNode(m->getFirstChild()); m != NULL;
       m = getNextElementNode(m)) {
    // debugLog(NGB_DICT_MGR, "_%s__%s_", XMLString::transcode(m->getNodeName()),
    //          getAttributeValue(m, g_xmlch_attribute_name).c_str());
    Avp *avp = getAvpByName(getAttributeValue(m, g_xmlch_attribute_name));
    if (avp == (Avp*)-1) {
      debugLog(NGB_DICT_MGR,
               "DictionaryManager::parseGroupAvp fail to get avp by Name");
    }
    gavp->addAvp(avp);
  }
  debugLog(NGB_DICT_MGR, "DictionaryManager::parseGroupAvp exit");
  return gavp;
}

std::string Avp::toString(int numOfIndent)
{
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  std::string myString = myIndent + type_ + std::string("::") + name_ +
    std::string("(") + Utils::intToString(length_) + std::string(":") +
    Utils::intToString(quantity_) + std::string(":") +
    Utils::intToString(deepth_) + std::string(") ") + signature_;
    return myString;
}

std::string GroupAvp::toString(int numOfIndent)
{
  std::string myIndent = Utils::makeDuplicate(INDENT, numOfIndent);
  std::string myString = myIndent + type_ + std::string("::") + name_ +
    std::string("(:") + Utils::intToString(quantity_) +
    std::string(":") + Utils::intToString(deepth_) + std::string(")\n");
  for (std::vector<Avp*>::iterator it = subAvpList_.begin();
       it != subAvpList_.end(); ++it) {
    myString += (*it)->toString(numOfIndent + 1) + std::string("\n");
  }
  return myString;
}

Avp* GroupAvp::findAvp(std::string &findName)
{
  debugLog(NGB_DICT_MGR,
           "GroupAvp::findAvp groupAvp(%s) enter", getName().c_str());
  Avp *avp = 0;
  for (std::vector<Avp*>::iterator ite = subAvpList_.begin();
       ite != subAvpList_.end(); ite++) {
    if ((*ite)->getType() == std::string("grouped")) {
      avp = ((GroupAvp*)(*ite))->findAvp(findName);
      if (avp != (Avp*)-1) {
        return avp;
      }
    } else {
      if ((*ite)->getName() == findName) {
        return (Avp*)(*ite);
      }
    }
  }
  return (Avp*)-1;
}
