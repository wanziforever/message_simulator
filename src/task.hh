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

#include <iostream>
#include <vector>
#include "run_item.hh"
#include "utils.hh"

class Task
{
public:
  Task();
  Task(std::string path);
  // main interface for run the task
  bool processTask();
  bool init();
  std::string getPath();
  
private:
  // read the task file and setup the item list which need to run
  bool setupRunItems();
  Utils::Path path_;
  std::vector<RunItem> items_;
};

