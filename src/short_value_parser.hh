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

#ifndef NGB_SHORT_VALUE_PARSER_H__
#define NGB_SHORT_VALUE_PARSER_H__

#include <string.h>
#include <stdio.h>
#include "value_parser.hh"
#include "log.hh"

// TODO; consider the compiler will have the margin indent
class ShortValueParser : public ValueParser
{
  // parse string format value to integer32 format
  bool parseAppToRaw(void *raw, int &len) {
    debugLog(NGB_VALUE_PARSER, "Integer32ValueParser::parseAppToRaw enter");
    if (!strlen(valueStr_)) {
      // if the value string is not provisioned, use NULL to fill instead
      debugLog(NGB_VALUE_PARSER, "Integer32ValueParser::parseAppToRaw "
               "input value is not provisioned, use NULL instead");
      memset(raw, 0, sizeof(short)*getLength());
      len = getLength() * sizeof(short);
      return true;
    }
    // integer type will ignore the length arribute
    short val = 0;
    int intval = 0;
    sscanf(valueStr_, "%d", &intval);
    *((short*)raw) = (short)intval;
    len += sizeof(short);
    return true;
  }

  bool parseRawToApp(void *app, int &len) {
    debugLog(NGB_VALUE_PARSER,
             "Integer32ValueParser::parseRawToApp enter");
    int i = 0;
    len = 0;
    short shortval;
    int intval;
    shortval = *((short*)valueStr_);
    intval = (int)shortval;
    sprintf((char*)app, "%d", intval);
    
    len += sizeof(short);
    return true;
  }
};

#endif
