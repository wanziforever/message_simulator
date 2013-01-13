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

#ifndef NGB_AVP_TYPE_H__
#define NGB_AVP_TYPE_H__

#include <sys/types.h>
#include <list>
#include <string.h>

typedef enum
{
  NGB_OCTECT_STRING_TYPE,
  NGB_UTF8_STRING_TYPE,
  NGB_IP_ADDRESS_TYPE,
  NGB_INTEGER32_TYPE,
  NGB_INTEGER64_TYPE,
  NGB_SHORT_TYPE,
} NGB_DATA_TYPE;

class AvpType
{
public:
  AvpType(const char *name, NGB_DATA_TYPE type, int size) {
    name_ = name;
    type_ = type;
    size_ = size;
  }

  const char* getName() { return name_; }
  NGB_DATA_TYPE getType() { return type_; }
  int getMinSize() { return size_; }
private:
  const char *name_;
  NGB_DATA_TYPE type_;
  int size_;
};

// no multi-thread consideration currently for global static member
class AvpTypeList
{
public:
  AvpTypeList();
  ~AvpTypeList();
  AvpType* search(const char*);
private:
  static std::list<AvpType*> avpt_l_;
  // indicates the number of instance of this class
  static int counter_;
};

#endif

