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

#ifndef NGB_VALUE_PARSER_H__
#define NGB_VALUE_PARSER_H__

#define VALUE_IS_TEXT 1
#define VALUE_IS_RAW  0

// parent class of all the value parser class
class ValueParser
{
public:
  ValueParser(char *value, int len = 0)
    : valueStr_(value), len_(len) { }

  ValueParser() : valueStr_(0), len_(0) {}
  ~ValueParser() {
    valueStr_ = 0;
  }

  virtual bool parseAppToRaw(void *raw, int &len) = 0;
  virtual bool parseRawToApp(void *app, int &len) = 0;

  void setValue(const char* value, bool flag) {
    valueStr_ = value;
    flag_ = flag;
  }

  const char* getValue() { return valueStr_; }
  int getFlag() { return flag_; }

  int getLength() { return len_; }
  void setLength(int len) { len_ = len; }
  
protected:
  const char *valueStr_;
  // to tell the value stored is raw or text
  // 1 for VALUE_IS_TEXT
  // 0 for VALUE_IS_RAW
  bool flag_;
  int len_;
};

#endif
