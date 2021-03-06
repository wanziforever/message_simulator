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

#include <fstream>
#include <unistd.h>
#include <sys/param.h>
#include <algorithm>
#include <sstream>
#include "log.hh"
#include "config_manager.hh"
#include "utils.hh"

using namespace std;
string ConfigManager::fileName_ = string("");
map<string, string> ConfigManager::paraDict_;

#define SEPARATOR "=:"
// parse a given line of string, parse it to name value pair, and put
// it to Map hash structure for later use, the separator here is defined
// by SEPARATOR macro.
bool ConfigManager::parseParameters(string &input)
{
  int pos = input.find_first_of(SEPARATOR);
  if (pos == string::npos) {
    return false;
  }
  string name = input.substr(0, pos);
  string value = input.substr(pos+2/*, toTheEnd*/);
  name = Utils::trim(name);
  value = Utils::trim(value);
  paraDict_[name] = value;
  return true;
}

void ConfigManager::init(const string configFile)
{
  debugLog(NGB_CONFIG_MGR, "ConfigManager::init enter...");
  fileName_ = configFile;
  setupParameters();
}

// read from the configration file, and get all the parameters, parse
// them to the name value paires, and put them into the map hash structure
void ConfigManager::setupParameters()
{
  char line[256];
  ifstream configStream(fileName_.c_str(), ifstream::in);
  while (configStream.getline(line, 256)) {
    string entry = string(line);
    entry = Utils::trim(entry);
    if (!entry.size() || Utils::isComments(entry)) {
      continue;
    }

    if (!parseParameters(entry)) {
      debugLog(NGB_ERROR, "ConfigManager::setupParameter::"
               "Fail to parse parameter for:\n%s", entry.c_str());
      continue;
    }
  }
}

// get the value string from the configration file by parameter name
string ConfigManager::getValueByName(const string &name)
{
  return paraDict_[name];
}

string ConfigManager::getTask()
{
  return getValueByName("task");
}

string ConfigManager::getDestAddress()
{
  return getValueByName("dest_address");
}

int ConfigManager::getDestPort()
{
  int port = 0;
  stringstream(getValueByName("dest_port")) >> port;
  return port;
}

int ConfigManager::getLocalPort()
{
  int port = 0;
  stringstream( getValueByName("local_port")) >> port;
  return port;
}

int ConfigManager::getExpectedTPS()
{
  int tps = 0;
  stringstream(getValueByName("expected_tps")) >> tps;
  return tps;
}

bool ConfigManager::isTcpHeaderInResponse()
{
  std::string flag;
  flag = getValueByName("tcp_header_in_response");
  transform(flag.begin(), flag.end(), flag.begin(), ::toupper);
  
  return (flag == "Y" ? true : false);
}

string ConfigManager::getCWD()
{
  char path[MAXPATHLEN];
  getcwd(path, MAXPATHLEN);
  return string(path);
}

void ConfigManager::print()
{
  
}

std::string ConfigManager::getDisplayData()
{
  char str[2048] = {0};
  sprintf(str,
          "Scenario task file : %s\n"
          "Local      Port    : %d\n"
          "Destination Address: %s\n"
          "Destination  Port  : %d",
          getTask().c_str(), getLocalPort(),
          getDestAddress().c_str(), getDestPort());
  return std::string(str);
}


