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

using namespace xercesc;
// when want to parse a tag, add a XMLCH type string here for convinent
// following XMLCH will not released forever, define just once, all the
// instance was initialized in the DictionaryManager::init() function
static const XMLCh *g_xmlch_command = NULL;
static const XMLCh *g_xmlch_avp = NULL;
static const XMLCh *g_xmlch_avpRule = NULL;
static const XMLCh *g_xmlch_attribute_name = NULL;
static const XMLCh *g_xmlch_attribute_code = NULL;
static const XMLCh *g_xmlch_attribute_type = NULL;
static const XMLCh *g_xmlch_attribute_length = NULL;
static const XMLCh *g_xmlch_attribute_quantity = NULL;

static XercesDOMParser::ValSchemes gValScheme = XercesDOMParser::Val_Auto;

bool Command::addAvp(Avp avp)
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
  *output = avps_[index];
  return true;
}

bool Command::hasAvp(std::string &findName)
{
  int index = 0;
  for(;index<numOfAvp_;++index) {
    if(avps_[index].getName() == findName) {
      return true;
    }
  }
  return false; // not found match
}

bool Command::findAvp(std::string &findName, Avp *output)
{
  assert(output);
  int index = 0;
  for(;index<numOfAvp_;++index) {
    if(avps_[index].getName() == findName) {
      *output = avps_[index];
      return true;
    }
  }
  return false;
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
  if (!XMLString::equals(m->getNodeName(), g_xmlch_attribute_type)) {
    return NULL;
  }
  return getAttributeValue(m, g_xmlch_attribute_name);
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

  g_xmlch_command = XMLString::transcode("command");
  g_xmlch_avp = XMLString::transcode("avp");
  g_xmlch_avpRule = XMLString::transcode("avprule");
  g_xmlch_attribute_name = XMLString::transcode("name");
  g_xmlch_attribute_code = XMLString::transcode("code");
  g_xmlch_attribute_type = XMLString::transcode("type");
  g_xmlch_attribute_length = XMLString::transcode("length");
  g_xmlch_attribute_quantity = XMLString::transcode("quantity");

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

bool DictionaryManager::getOneCommand(const DOMNode *n, Command *output)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::getOneCommand enter...");
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
 
    Avp avp;
    if (!getAvpByName(getAttributeValue(m, g_xmlch_attribute_name), &avp)) {
      debugLog(NGB_DICT_MGR,
               "DictionaryManager::getOneCommand fail to get avp by avpName");
      return false;
    }
    avp.setQuantity(getAttributeValue(m, g_xmlch_attribute_quantity));
    command->addAvp(avp);
  }
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

bool DictionaryManager::getAvpByName(const std::string &name, Avp *output)
{
  debugLog(NGB_DICT_MGR, "DictionaryManager::getAvpByName enter...");
  assert(output);
  Avp *avp = output;
  DOMDocument *doc = parser_->getDocument();
  DOMNodeList *avpNodeList = doc->getElementsByTagName(g_xmlch_avp);
  for (int i = 0; i < avpNodeList->getLength(); i++) {
    DOMNode *n = avpNodeList->item(i);
    if (getAttributeValue(n, g_xmlch_attribute_name) != name) {
      continue;
    }
    avp->setName(getAttributeValue(n, g_xmlch_attribute_name));
    avp->setCode(getAttributeValue(n, g_xmlch_attribute_code));
    avp->setLength(getAttributeValue(n, g_xmlch_attribute_length));
    // avptype is a sub tag of the avp node, use getAvpType() to get it
    avp->setType(getAvpType(n));
    return true;
  }
  debugLog(NGB_DICT_MGR,
           "DictionaryManager::getAvpByName cannot get \"%s\"", name.c_str());
  return false;// not found matched avp
}

