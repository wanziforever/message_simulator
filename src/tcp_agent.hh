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

#ifndef NGB_TCP_AGENT_H__
#define NGB_TCP_AGENT_H__

#include <iostream>
#include <queue>
#include "message.hh"

class TcpAgent
{
public:
  TcpAgent() : socket_(0), totalCounter_(0) {}
  ~TcpAgent();
  bool init(int localPort);
  bool tcpConnect(const char *ip, int port);
  int sendMsg(char *msg, int len);
  int getSocket() { return socket_; }
  int getQueueSize();
  Message* receive();
  unsigned long long getTotalCounter() { return totalCounter_; }
  void clearQueue();
private:
  int writeData(int fd, char *buf, int size);
  int readData(char *buf, int size);
  bool startReceiver();
  bool registerReceiver(int fd);
  void* receiverThreadFunc();
  bool getMsgWithoutTcpHeader(Message*);
  bool getMsgWithTcpHeader(Message*);

  // members
  int socket_;
  int epoolfd_;
  unsigned long long totalCounter_;
  std::queue<Message*> recMsgQueue_;
};

typedef struct tcp_msg_hdr {
  unsigned short ver;
  unsigned char reserved[2];
  unsigned int data_len;
  unsigned int session_id;
  unsigned int reserve;
  unsigned char padding[8];
} tcp_msg_hdr_t;

struct tcp_handle {
  int fd;
  unsigned int src_ip;
  unsigned int src_port;
  int fd2;
  unsigned int dst_ip;
  unsigned int dst_port;

  struct timeval ts;
  unsigned int scrhdr_len;

  struct tcp_msg_hdr msghdr;
  unsigned int hdr_len;
  char *data_buf;
  unsigned int data_len;
  unsigned int buf_len;
  time_t qtime;
};

#endif

