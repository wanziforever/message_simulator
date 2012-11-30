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

  }
  bool parseRawToApp(void *raw, int &len) {
    debugLog(NGB_VALUE_PARSER,
             "Integer64ValueParser::parseRawToApp enter");
  }
};

#endif
