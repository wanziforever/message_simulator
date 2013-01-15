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

#include "avp_type.hh"

std::list<AvpType*> AvpTypeList::avpt_l_;
int AvpTypeList::counter_;

AvpTypeList avpTypeList;

// the size as 0 means the size is variable
AvpTypeList::AvpTypeList()
{
  if(counter_++ > 0)
    return;
  avpt_l_.push_back(new AvpType("OctectString", NGB_OCTECT_STRING_TYPE, 0));
  avpt_l_.push_back(new AvpType("UTF8String",   NGB_UTF8_STRING_TYPE, 0));
  avpt_l_.push_back(new AvpType("IPAddress",    NGB_IP_ADDRESS_TYPE, 0));
  avpt_l_.push_back(new AvpType("Integer32",   NGB_INTEGER32_TYPE, 4));
  avpt_l_.push_back(new AvpType("Integer64",   NGB_INTEGER64_TYPE, 8));
  avpt_l_.push_back(new AvpType("Short",   NGB_SHORT_TYPE, 2));
  avpt_l_.push_back(new AvpType("byte",   NGB_BYTE_TYPE, 1));
}

AvpTypeList::~AvpTypeList()
{
  std::list<AvpType*>::iterator i;
  if (--counter_ == 0) {
    for (i = avpt_l_.begin(); i != avpt_l_.end(); delete *i, i++);
  }
}

// should have lock when support multi-thread
AvpType* AvpTypeList::search(const char *type)
{
  std::list<AvpType*>::iterator i;
  for (i = avpt_l_.begin(); i != avpt_l_.end(); i++) {
    if (strcmp((*i)->getName(), type) == 0) {
      return *i;
    }
  }
  return NULL;
}
