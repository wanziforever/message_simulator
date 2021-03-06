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

#ifndef NGB_UTILS_H__
#define NGB_UTILS_H__
#define UTILS_NAMESPACE_START namespace Utils {
#define UTILS_NAMESPACE_END }

#include <iostream>

UTILS_NAMESPACE_START

class Path
{
public:
  // constructor, if the input path is the a relative path like:
  // ./XXX.yy, will convert it to XXX.yy
  Path(std::string path) : path_(path) {
    if (path_.compare(0, 2, "./")) {
      return;
    }
    path_ = path_.substr(2);
  }
  // define constructor for vector::push_back
  Path(const Path &path) {
    path_ = path.path_;
  }

  Path() {}
  
  inline bool isRelative() {
    return path_.compare(0, 1, "/");
  }
  
  Path& operator+=(const Path &path) {
    path_ = path_ + "/" + path.path_;
    return *this;
  }

  Path& operator=(const Path &path) {
    path_ = path.path_;
    return *this;
  }

  Path operator+(const Path &path) {
    Path ret(path_+"/"+path.path_);
    return ret;
  }
  
  char* c_str() {
    path_.c_str();
  }
  
  std::string toString() {
    return path_;
  }
private:
  std::string path_;
};


std::string& LTrim(std::string&);
std::string& RTrim(std::string&);
std::string& trim(std::string&);
// tell whether the line is a comments line
bool isComments(std::string&, std::string comment="#");
bool isRelativePath(const std::string&);;
int binaryToAscii(char*, int, char*);
std::string intToString(int number);
std::string longlongToString(long long number);
int stringToInt(std::string number);
long long stringToLongLong(std::string number);
std::string makeDuplicate(const std::string &str, int x);
unsigned long long getTimeOfDayinUsec();
bool fileExist(std::string&);
UTILS_NAMESPACE_END
#endif
