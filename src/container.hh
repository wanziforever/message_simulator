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
  AvpEntry() : avpCode_(0), length_(0), value_(""), avpName_(""),
               quantity_(0), type_(""){}
  AvpEntry(const AvpEntry &avp) {
    avpName_ = avp.avpName_;
    value_ = avp.value_;
    avpCode_ = avp.avpCode_;
    length_ = avp.length_;
    type_ = avp.type_;
    quantity_ = avp.quantity_;
  }
  bool parseAppToRaw(char *raw, int &len);
  bool parseRawToApp(char *raw, int &len);
  // getter and setter method
  std::string getName() { return avpName_; }
  // TODO; the getValue should be enhanced to return one value for
  // one quantity, some format of value should be designed
  std::string getValue() { return value_; }
  std::string getType() { return type_; }
  int getCode() { return avpCode_; }
  int getLength() { return length_; }
  int getQuantity() { return quantity_; }

  void setName(std::string name) { avpName_ = name; }
  void setValue(std::string val) { value_ = val; }
  void setType(std::string tp) { type_ = tp; }
  void setCode(int code) { avpCode_ = code; }
  // if the length is configured to 0, will convert it to 1
  void setLength(int len) {
    length_ = (len == 0) ? 1 : len; }
  // if the quantity is configured to 0, will convert it to 1
  void setQuantity(int quantity) {
    quantity_ = (quantity == 0) ? 1 : quantity;}
private:
  std::string avpName_;
  std::string value_;
  int avpCode_;
  int length_;
  int quantity_;
  std::string type_;
};

class Container
{
public:
  Container() {}
  ~Container() {}
  // add the avp to avp list, only set the name and value
  // field for the avpEntry structure, and will full fill
  // the structure by fillAvpsWithTypesFromDictionary() menthod
  bool addAvp( std::string name, std::string value );
  bool addAvp(std::string name, std::string type, int len,
              int quantity);
  // full fill the avps for all ones in avpList, get full
  // information from dictionary manager object
  bool fillAvpsWithTypesFromDictionary(std::string cmdName);
  // create each avp entries in the container with name and types
  // from dictionary document, the container normally has no entry
  // before calling this menthod
  bool createAvpsFromDictionary(std::string cmdName);
  void print();
  int parseAppToRaw(char *raw);
  int parseRawToApp(char *raw);

private:
  std::vector<AvpEntry> avpList_;
};

#endif
  
