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
// some definition of this class, copy from older version, and paste here
//

#ifndef NGB_DICTIONARY_MANAGER_H__
#define NGB_DICTIONARY_MANAGER_H__

#include <string>
#include <cassert>
#include <sstream>
#include <xercesc/util/XercesDefs.hpp>

// class reference to the avp tag in dictionary document
class Avp
{
public:
  // constructor
  Avp() : name_(std::string("")), type_(std::string("")),
          code_(0), length_(0), quantity_(0) {}
  Avp(std::string name, int code)
    :name_(name), code_(code), type_(std::string("")),
     length_(0), quantity_(0) {}
  Avp(const Avp &avp) {
    name_ = avp.name_;
    type_ = avp.type_;
    code_ = avp.code_;
    length_ = avp.length_;
    quantity_ = avp.quantity_;
  }
  ~Avp() {}

  // getter and setter menthod
  std::string getName() { return name_; }
  std::string getType() { return type_; }
  int getCode() { return code_; }
  int getLength() { return length_; }
  int getQuantity() { return quantity_; }   
  void setName( const std::string name ) { name_ = name; }
  void setType( const std::string type ) { type_ = type; }
  void setType( const char *type ) { type_ = std::string(type); }
  void setCode( const int code ) { code_ = code; }
  void setCode( const std::string code ) {
    std::stringstream(code) >> code_;
  }
  void setLength(const int len) { length_ = len; }
  void setLength(const std::string len) {
    if (len.size() == 0) {
      length_ = 0;
    }
    std::stringstream(len) >> length_;
  }

  void setQuantity(const int quantity) { quantity_ = quantity; }
  void setQuantity(const std::string quantity) {
    if (quantity.size() == 0) {
      quantity_ = 0;
    }
    std::stringstream(quantity) >> quantity_;
  }

  Avp& operator = (const Avp &other) {
    this->name_ = other.name_;
    this->type_ = other.type_;
    this->code_ = other.code_;
    this->length_ = other.length_;
    this->quantity_ = other.quantity_;
    return *this;
  }
  
private:
  std::string name_;
  std::string type_;
  int         code_;
  int         length_;
  int         quantity_;
};

// class reference to the command tag in dictionary document
#define MAX_AVP_NUM_PER_COMMAND 40
class Command
{
public:
  // constructor
  Command(int size=MAX_AVP_NUM_PER_COMMAND) : size_(size) {
    numOfAvp_ = 0;
    avps_ = new Avp[size_];
  }
  Command(std::string name, int code, int size=MAX_AVP_NUM_PER_COMMAND)
    : name_(name),code_(code),size_(size) {
    avps_ = 0;
    numOfAvp_ = 0;
    avps_ = new Avp[size_];
  }

  ~Command() {
    if(!avps_) 
      delete[] avps_;
    avps_ = 0;
  }

  // getter and setter menthod
  std::string getName() { return name_; }
  int  getCode() { return code_; }
  void setName( const std::string name ) { name_ = name; }
  void setName( const char *name ) { name_ = std::string(name); }
  void setCode( const int code ) { code_ = code; }
  void setCode( const std::string code ) {
    std::stringstream(code) >> code_;
  }

  // add one avp to the current command
  bool addAvp( Avp avp );

  // check whether the command has a given avp by name
  bool hasAvp( std::string &findName );


  // find and get the avp by the given name, result to assigned
  // to output argument, there is no performance consideration
  // for find a element in a list container, if there is a
  // performance requirement, use a MAP or SET container instead
  bool findAvp(std::string &findName, Avp *output);

  // get the avp by index 
  bool getAvpByIndex(int index, Avp *output);

  // get the number of avps for the command
  int getNumOfAvp() {
    return size_;
  }
  Avp   *avps_;
  int    size_;
private:
  std::string name_;
  int    code_;
  int    numOfAvp_;
};

XERCES_CPP_NAMESPACE_BEGIN
class DOMNode;
class XercesDOMParser;
XERCES_CPP_NAMESPACE_END
class DOMTreeErrorReporter;

class DictionaryManager
{
public:
  // constructor
  DictionaryManager();
  // deconstructor
  ~DictionaryManager();
  // init the instance with the given input XML document
  bool init(const std::string &dictFile);
  // get the number of command definitions from the XML docuemnt
  int getNumOfCommand();
  // get the structured for all commands from dictionary document,
  // the output argument is a pointer, which point to the memory
  // allocated by caller, caller should firstly use getNumOfCommand
  // to get the size of memory
  bool getAllCommands(Command *output);
  // get the number of avp definitions from the XML document
  int getNumOfAvp();
  bool getCommandByName(std::string name, Command *output);

private:
  bool getOneCommand(const xercesc::DOMNode *n, Command *output);
  // get number of tag definitions by given name in XML document
  int getNumOfTagByName(const XMLCh *name);
  // get the avp tag by the given avp name
  bool getAvpByName(const std::string &name, Avp *output);
  // get root dictionary DOMNode, this is called by internal menthod
  xercesc::DOMNode* getRootDictionaryNode();

  // members
  xercesc::XercesDOMParser *parser_;
  DOMTreeErrorReporter *errorReporter_;
};

#endif
