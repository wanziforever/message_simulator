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

#ifndef NGB_CONFIG_MANAGER_H__
#define NGB_CONFIG_MANAGER_H__
#include <map>

typedef struct para_s
{
  std::string name;
  std::string value;
} para_t;
  
class ConfigManager
{
public:
  static void init(const std::string configFile);
  static std::string getValueByName(const std::string &name);
  static void print();
private:
  static void setupParameters();
  static bool parseParameters(std::string &input);
  static std::string fileName_;
  static std::map<std::string, std::string> paraDict_;

// some defined interface for other component
public:
  // request definition for common use
  static std::string getCWD();
  // request definition for main function
  static std::string getTask();
  static std::string getDestAddress();
  static int getDestPort();
  static int getLocalPort();
};

#endif
