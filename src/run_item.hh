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

#ifndef NGB_RUN_ITEM_H__
#define NGB_RUN_ITEM_H__

#include "utils.hh"

class RunItem
{
public:
  // constructor will get a line of string as the parameters list
  // the parameters will be parsed internal by RunItem
  RunItem(std::string parameters);
  // define constructor for vector::push_back
  RunItem(const RunItem &item) {
    mode_ = item.mode_;
    timeOut_ = item.timeOut_;
    pathOfMessage_ = item.pathOfMessage_;
  }
  
  ~RunItem() {}
  bool processItem();
private:
  // parse the input line of string into parameters
  bool setupRunItems(std::string line);
  // read and parse message from disk, send message out
  bool sendMessage();
  // receive message from message queue, and parse it to readable
  // format, save the raw and readable format data to disk
  bool receiveMessage();
  std::string mode_; // Send or Receive
  int timeOut_;
  Utils::Path pathOfMessage_;
};

#endif
