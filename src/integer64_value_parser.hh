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

#ifndef NGB_INTEGER64_VALUE_PARSER_H__
#define NGB_INTEGER64_VALUE_PARSER_H__

#include "value_parser.hh"
#include "log.hh"

class Integer64ValueParser : public ValueParser
{
  bool parseAppToRaw(void *raw, int &len) {
    debugLog(NGB_VALUE_PARSER,
             "Integer64ValueParser::parseAppToRaw enter");
    if (!strlen(valueStr_)) {
      debugLog(NGB_VALUE_PARSER, "Integer64ValueParser::parseAppToRaw "
        "intput value is not provisioned, use NULL instead");
      memset(raw, 0, sizeof(long long) * getLength());
      len = getLength() * sizeof(long long);
      return true;
    }
    int i = 0;
    for (; i < getLength(); i++) {
      sscanf(valueStr_ + i * sizeof(long long), "%lld", (long long *)raw);
      len += sizeof(long long);
    }
    return true;
  }
  
  bool parseRawToApp(void *app, int &len) {
    debugLog(NGB_VALUE_PARSER,
             "Integer64ValueParser::parseRawToApp enter");
    int i = 0;
    len = 0;
    for (; i < getLength(); i++) {
      sprintf((char*)app, "%lld",
                    *((long long*)(valueStr_ + i * sizeof(long long))));
      len += sizeof(long long);
    }
    return true;
  }
};

#endif
