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

#ifndef NGB_CONTAINER_H__
#define NGB_CONTAINER_H__

#include <vector>
#include <iostream>

class AvpEntry
{
public:
  AvpEntry() : type_(""), value_(""), avpName_(""), avpCode_(0),
               length_(1), quantity_(1), data_(0) {} 
  virtual bool parseAppToRaw(char *raw, int &len);
  virtual bool parseRawToApp(char *raw, int &len);
  virtual std::string toString(int numOfIndent = 0);
  void setValue(std::string val) { value_ = val; }
  std::string getValue() { return value_; }
  void setName(std::string name) { avpName_ = name; }
  std::string getName() { return avpName_; }
  void setType(std::string type) { type_ = type; }
  std::string getType() { return type_; }
  void setCode(int code) { avpCode_ = code; }
  int getCode() { return avpCode_; }
  void setLength(int len) {
    length_ = (len == 0) ? 1 : len; 
  }
  int getLength() { return length_; }
  void setQuantity(int quantity) {
    quantity_ = (quantity == 0) ? 1 : quantity;
  }
  int getQuantity() { return quantity_; }
  void setDeepth(int deepth) { deepth_ = deepth; }
  int getDeepth() { return deepth_; }

  void setSignature(std::string signature) { signature_ = signature; }
  std::string getSignature() { return signature_; }
  std::string getParentSignature();
  char* getDataPointer() {return data_; }
protected:
  std::string type_;
  std::string value_;
  std::string avpName_;
  std::string signature_;
  int avpCode_;
  int length_;
  int quantity_;
  int deepth_;
  char* data_; // pointer in the raw message for this avp
};

class EntryContainer
{
public:
  EntryContainer() {}
  ~EntryContainer() { // remember to delete the  avps in the container
  }
  // add the avp to avp list, only set the name and value
  // field for the avpEntry structure, and will full fill
  // the structure by fillAvpsWithTypesFromDictionary() menthod
  virtual bool addAvp( std::string name, std::string value );
  virtual bool addAvp(std::string name, std::string type, int len,
              int quantity);
  virtual bool parseAppToRaw(char *raw, int &len) = 0;
  virtual bool parseRawToApp(char *raw, int &len) = 0;
  virtual std::string toString(int numOfIndent = 0);
  
  EntryContainer* appendContainer(std::string name);
  AvpEntry* appendAvp(std::string name, std::string value);
  AvpEntry* appendAvp(std::string name, std::string type, int len,
                      int quantity);
  void appendAvp(AvpEntry* avp) {
    avpList_.push_back(avp);
  }
  std::vector<AvpEntry*> avpList_;
};

class GavpEntry : public AvpEntry, public EntryContainer
{
public:
  GavpEntry() {}
  ~GavpEntry() {}
  bool parseAppToRaw(char *raw, int &len);
  bool parseRawToApp(char *raw, int &len);
  bool addAvp( std::string name, std::string value );
  bool addAvp(std::string name, std::string type, int len,
              int quantity);
  std::string toString(int numOfIndent);
};

class MessageEntryContainer : public EntryContainer
{
public:
  MessageEntryContainer() {}
  ~MessageEntryContainer() {
    //remeber to delete all the avps in the container
  }
  
  // full fill the avps for all ones in avpList, get full
  // information from dictionary manager object
  // bool fillAvpsWithTypesFromDictionary(std::string cmdName);
  // create each avp entries in the container with name and types
  // from dictionary document, the container normally has no entry
  // before calling this menthod
  // bool createAvpsFromDictionary(std::string cmdName);
  void print();
  bool parseAppToRaw(char *raw, int &len);
  bool parseRawToApp(char *raw, int &len);

};

#endif
