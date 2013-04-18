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

#include <sstream>
#include <sys/time.h>
#include "log.hh"
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
// convert a number to Hex string, the number should stored in
// a char type varaible
char toHex(char input)
{
  if (input > 0 && input < 10) {
    return ('0' + input);
  } else if (input >=10 && input < 16) {
    return ('a' + input - 10);
  } else {
    return '0';
  }
}

void binaryToHexForOneChar(char c, char *output)
{
  char input_char = c;
  char hig = (input_char >> 4) & 0x0f;
  char low = (input_char) &  0x0f;
  *output = toHex(hig);
  *(output+1) = toHex(low);
}

// convert the binary data to a readable text format string with
// the Hex string
int binaryToAscii(char *raw, int len, char *output)
{
  int pos_in_raw = 0;
  int pos_in_output = 0;
  for (; pos_in_raw < len; pos_in_raw++) {
    char raw_char = *(raw + pos_in_raw);
    binaryToHexForOneChar(raw_char, output + pos_in_output);
    pos_in_output += 2;
  }
  return pos_in_output;
}

std::string intToString(int number)
{
  std::stringstream oss;
  oss << number;
  return oss.str();
}

int stringToInt(std::string number)
{
  int ret = 0;
  std::istringstream ss(number);
  ss >> ret;
  return ret;
}

std::string makeDuplicate(const std::string &str, int x)
{
  std::string newstr;
  if (x < 0) return str;
  if (x == 0) return std::string("");
  newstr.reserve(str.length() * x);
  int y = 0;
  while (y < x) {
    newstr.append(str);
    y++;
  }
  return newstr;
}

unsigned long long getLlongLongRandom()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long long time = tv.tv_sec * 1000000 + tv.tv_usec;
  return time;
}

UTILS_NAMESPACE_END
