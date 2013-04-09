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

#ifndef NGB_IPADDRESS_VALUE_PARSER_H__
#define NGB_IPADDRESS_VALUE_PARSER_H__

#include <string.h>
#include <stdio.h>
#include "value_parser.hh"
#include "log.hh"

// TODO; consider the compiler will have the margin indent
class IPaddressValueParser : public ValueParser
{
  // parse string format value to integer32 format
  bool parseAppToRaw(void *raw, int &len) {
    debugLog(NGB_VALUE_PARSER, "IPaddressValueParser::parseAppToRaw enter");
    if (!strlen(valueStr_)) {
      // if the value string is not provisioned, use NULL to fill instead
      debugLog(NGB_VALUE_PARSER, "IPaddressValueParser::parseAppToRaw "
               "input value is not provisioned, use NULL instead");
      memset(raw, 0, sizeof(int));
      len = sizeof(int);
      return true;
    }
    // integer type will ignore the length arribute
    short val = 0;
    int ipbytes[4];
    char tail = 0;
    int fields;
    fields = sscanf(valueStr_, "%d.%d.%d.%d %c",
                    &ipbytes[3], &ipbytes[2], &ipbytes[1], &ipbytes[0], &tail);
    if (fields != 4 || tail != 0) {
      debugLog(NGB_VALUE_PARSER,
               "IPaddressValueParser::parseAppToRaw fail to convert ip string");
      memset(raw, 0, sizeof(int));
      len = sizeof(int);
      return true;
    }
    *((unsigned int*)raw) =
      ipbytes[0] | ipbytes[1] << 8 | ipbytes[2] << 16 | ipbytes[3] << 24;
    len += sizeof(int);
    return true;
  }

  bool parseRawToApp(void *app, int &len) {
    debugLog(NGB_VALUE_PARSER,
             "IPaddressValueParser::parseRawToApp enter");
    int i = 0;
    len = 0;
    unsigned int numericval;
    numericval = *((unsigned int*)valueStr_);
    char ips[20] = {0};
    sprintf(ips, "%d.%d.%d.%d",
            (numericval >> 24) & 0xFF,
            (numericval >> 16) && 0xFF,
            (numericval >> 8) & 0xFF,
            numericval & 0xFF );
    strncpy((char*)app, ips, 20);
    len += sizeof(int);
    return true;
  }
};

#endif
