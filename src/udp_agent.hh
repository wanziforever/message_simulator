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
// 
// This tool is designed to simulate the internal message for
// NGB project unit testing, the message body can be configured
// in the specified XML document.

#ifndef NGB_UDP_AGENT_H__
#define NGB_UDP_AGENT_H__

#include <iostream>
#include <queue>
#include "message.hh"

class UdpAgent
{
public:
  UdpAgent() : socket_(0) {};
  ~UdpAgent();
  bool init(int localPort);
  bool sendMsg(const char *ip, int port, char *msg, int len);
  int getSocket() { return socket_; }
  int getQueueSize();
  Message receive();
private:
  bool startReceiver();
  bool registerReceiver(int fd);
  void* receiverThreadFunc();
  int socket_;
  int epollfd_;
  std::queue<Message> recMsgQueue_;
};

#endif
