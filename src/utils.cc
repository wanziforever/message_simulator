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

// define the util tools for NGB project

#include "utils.hh"

UTILS_NAMESPACE_START

using namespace std;

string& LTrim(string &str)
{
  if (str.find_first_not_of(" \n\r\t")!=string::npos) {
    str = str.substr(str.find_first_not_of(" \n\r\t"));
  }
  return str;
}

string& RTrim(string &str)
{
  if (str.find_first_not_of(" \n\r\t") != string::npos) {
    str = str.substr(0,str.find_last_not_of(" \n\r\t")+1); 
  }
  return str;
}

string& trim(string &str)
{
  return LTrim(RTrim(str));
}

bool isComments(string &str, string comment)
{
  return !str.compare(0, comment.size(), comment);
}

void binaryToAsciiForOneChar(char *input, char *output)
{
  
}

// convert the binary data to a readable text format string with
// the Hex string
void binaryToAscii(char *raw, int len, char *output)
{
  int pos = 0;
  
}


UTILS_NAMESPACE_END