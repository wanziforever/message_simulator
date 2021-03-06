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

#ifndef NGB_UTF8_VALUE_PARSER_H__
#define NGB_UTF8_VALUE_PARSER_H__

#include <string.h>
#include "value_parser.hh"

// currently no convertion implementation for Octect type string
// the output memory should be allocated before
// NOTE:
//   the value string should not contain the valid NULL charactors
//   the value convertion will stopped when met the NULL, further
//   requirement need further implementation

class Utf8ValueParser : public ValueParser
{
public:
  // the len argument is the final length that how many data copied to
  // raw buffer, although it is possible the value provided is longer
  // then the len argument, there will be no NULL to be put behind, so
  // that the raw buffer should initialized with 0
  // currently the valueStr_ pointing to hunamn readable string which
  // read from message file, and use setValue() to valueStr_
  bool parseAppToRaw(void *raw, int &len) {
    debugLog(NGB_VALUE_PARSER, "Utf8ValueParser::parseAppToRaw enter");
    if (flag_ == VALUE_IS_RAW) // already raw
      return true;
    strncpy((char *)raw, valueStr_, getLength());
    len = getLength();
    return true;
  }
  // parse the input raw data to human readable value string
  bool parseRawToApp(void *app, int &len) {
    debugLog(NGB_VALUE_PARSER, "Utf8ValueParser::parseRawToApp enter");
    if (flag_ == VALUE_IS_TEXT) // already text
      return true;
    strncpy((char *)app, valueStr_, getLength());
    len = getLength();
    return true;
  }
};

#endif
