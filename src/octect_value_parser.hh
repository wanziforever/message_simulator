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

#ifndef NGB_OCTECT_VALUE_PARSER_H__
#define NGB_OCTECT_VALUE_PARSER_H__

#include <string.h>
#include "value_parser.hh"
#include "log.hh"

// normally the octect string should convert UTF-8 to Hex binary
// currently no convertion implementation for Octect type string
// the raw memory should be allocated before
// NOTE:
//   the NULL characters in the Octect type data is considered as
//   valide data, and the convertion raw will strictly use the
//   length provide by caller

class OctectValueParser : public ValueParser
{
  
  bool parseAppToRaw(void *raw, int &len) {
    debugLog(NGB_VALUE_PARSER,
             "OctectValueParser::parseAppToRaw enter");
    if (flag_ == VALUE_IS_RAW) // already raw
      return true;
    strncpy((char*)raw, valueStr_, getLength());
    len = getLength();
    return true;
  }

  bool parseRawToApp(void *app, int &len) {
    debugLog(NGB_VALUE_PARSER,
             "OctectValueParser::parseRawToApp enter");
    if (flag_ == VALUE_IS_TEXT) // already text
      return true;
    strncpy((char*)app, valueStr_, getLength());
    len = getLength();
    return true;
  }
};

#endif
