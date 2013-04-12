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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "tcp_agent.hh"
#include "log.hh"


typedef void* (TcpAgent::*Thread2Ptr)(void);
typedef void* (*PthreadPtr)(void*);

TcpAgent::~TcpAgent()
{
  if (socket_)
    close(socket_);
}

bool TcpAgent::init(int localPort)
{
  debugLog(NGB_TCP_AGENT,
           "TcpAgent::init enter with local port(%d)", localPort);
  if ((socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    debugLog(NGB_TCP_AGENT, "TcpAgent::int fail to create TCP socket");
    return false;
  }
  int optval = 1;
  setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  struct sockaddr_in src_addr;
  bzero((void *)&src_addr, sizeof(src_addr));
  src_addr.sin_family = AF_INET;
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  src_addr.sin_port = htons(localPort);
  if (bind(socket_, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
    debugLog(NGB_TCP_AGENT, "TcpAgent::init fail to bind local");
    return false;
  }
  return true;
}

bool TcpAgent::tcpConnect(const char* ip, int port)
{
  struct sockaddr_in  dest_addr;
  if (!ip) {
    debugLog(NGB_TCP_AGENT, "TcpAgent::connect invalid ip pointer");
    return false;
  }
  bzero(&dest_addr, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_addr.s_addr = inet_addr(ip);
  dest_addr.sin_port = htons(port);
  int con_ret = 0;
  con_ret = connect(socket_, (sockaddr*)&dest_addr, sizeof(dest_addr));
  if (con_ret != 0) {
    debugLog(NGB_TCP_AGENT,
             "TcpAgent::connect fail to connect %s, ret %d", ip, con_ret);
    return false;
  }
  debugLog(NGB_TCP_AGENT, "TcpAgent::connection established successfully");

  registerReceiver(socket_);
  
  startReceiver();

  return true;
}

int TcpAgent::sendMsg(char *msg, int len)
{
  int version = 0;
  tcp_msg_hdr_t hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ver = htons(version);
  hdr.data_len = htonl(sizeof(tcp_msg_hdr_t) + len);
  int result = 0;
  result = writeData(socket_, (char*)&hdr, sizeof(hdr));
  if (result != sizeof(hdr)) {
    debugLog(NGB_TCP_AGENT, "TcpAgent::sendMg fail to send header");
    return false;
  }
  result = writeData(socket_, msg, len);
  if (result != len) {
    debugLog(NGB_TCP_AGENT, "TcpAgent::snedMsg fail to send body");
    return false;
  }
  return result;
}

int TcpAgent::writeData(int fd, char* buf, int size)
{
  int offset = 0;
  int len_send = 0;
  int left = size;
  while (left > 0) {
    len_send = write(socket_, buf + offset, left);
    if (len_send < 0) {
      debugLog(NGB_TCP_AGENT, "TcpAgent::writeData fail to send message");
      return false;
    }
    left -= len_send;
    offset += len_send;
  }
  return offset;
}

int TcpAgent::readData(int fd, char *buf, int size)
{
  fd_set fdsr;
  int maxSock = socket_;
  struct timeval tv;
  int ret = 0;
  int offset = 0;
  int len_read = 0;
  int left = size;
  
  while (left > 0) {
    FD_ZERO(&fdsr);
    FD_SET(socket_, &fdsr);
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    ret = select(maxSock + 1, &fdsr,  NULL, NULL, &tv);
    if (ret < 0) {
      debugLog(NGB_TCP_AGENT, "TcpAgent::readData fail to select");
      break;
    } else if (ret == 0) {
      // timeout
      continue;
    }
    debugLog(NGB_TCP_AGENT, "TcpAgent::readData return value is %d", ret);
    len_read = read(fd, buf + offset, left);
    if (len_read < 0) {
      debugLog(NGB_TCP_AGENT, "TcpAgent::readData fail to read");
      continue;
    }
    if (len_read == 0) {
      debugLog(NGB_TCP_AGENT, "TcpAgent::readData read zero data");
      continue;
    }
    offset += len_read;
    left -= len_read;
  }
  return offset;
}

int TcpAgent::getQueueSize()
{
  return recMsgQueue_.size();
}

Message TcpAgent::receive()
{
  if (!recMsgQueue_.empty()) {
    Message msg;
    msg = recMsgQueue_.front();
    recMsgQueue_.pop();
    return msg;
  }
  return *(Message *)0;
}

bool TcpAgent::startReceiver()
{
  pthread_t thread;
  pthread_attr_t thread_attr;

  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  
  Thread2Ptr t = &TcpAgent::receiverThreadFunc;
  PthreadPtr p = *(PthreadPtr*)&t;
  pthread_create(&thread, &thread_attr, p, this);

  return 0;
}

bool TcpAgent::registerReceiver(int fd)
{
  return true;
}

void* TcpAgent::receiverThreadFunc()
{
  tcp_msg_hdr_t hdr;
  Message msg;
  while (1) {
    int len = readData(socket_, (char*)&hdr, sizeof(hdr));
    if (len != sizeof(hdr)) {
      debugLog(NGB_TCP_AGENT,
               "TcpAgent::receiverThreadFunc fatal network error");
      return (void *)false;
    }
    int ver = ntohs(hdr.ver);
    int payload_size = ntohl(hdr.data_len) - len;
    if (payload_size <= 0) {
      debugLog(NGB_TCP_AGENT,
               "TcpAgent::receiverThreadFunc invalid payload size");
      return (void *)false;
    }
    int read_size = readData(socket_, msg.getRawPtr(), payload_size);
    if (read_size != payload_size) {
      debugLog(NGB_TCP_AGENT,
               "TcpAgent::receiverThreadFunc faatal network error");
      return (void *)false;
    }
    recMsgQueue_.push(msg);
  }
}

