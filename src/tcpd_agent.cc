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
#include "tcpd_agent.hh"
#include "config_manager.hh"
#include "log.hh"


typedef void* (TcpdAgent::*Thread2Ptr)(void);
typedef void* (*PthreadPtr)(void*);

TcpdAgent::~TcpdAgent()
{
  if (socket_)
    close(socket_);
}

bool TcpdAgent::init(int localPort)
{
  debugLog(NGB_TCP_AGENT,
           "TcpAgent::init enter with local port(%d)", localPort);
  if ((socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    debugLog(NGB_ERROR, "TcpAgent::int fail to create TCP socket");
    return false;
  }
  int optval = 1;
  setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  struct linger so_linger;
  so_linger.l_onoff = 1;
  so_linger.l_linger = 0;
  setsockopt(socket_, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
  struct sockaddr_in src_addr;
  bzero((void *)&src_addr, sizeof(src_addr));
  src_addr.sin_family = AF_INET;
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  src_addr.sin_port = htons(localPort);
  if (bind(socket_, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
    debugLog(NGB_ERROR, "TcpAgent::init fail to bind local");
    return false;
  }
  return true;
}


bool TcpdAgent::tcpListen()
{
  // only accept one connect on connection
  debugLog(NGB_TCP_AGENT, "TcpdAgent::tcpListen enter");
  if (listen(socket_, 1)) {
    debugLog(NGB_ERROR, "TcpdAgent::tcpListen fail to listen");
    return false;
  }
  debugLog(NGB_TCP_AGENT,
           "TcpdAgent::tcpListen got one connection request");
  struct sockaddr_in client_addr;
  int new_socket = 0;
  socklen_t len = sizeof(client_addr);
  new_socket = accept(socket_,
                      (struct sockaddr*)&client_addr,
                      &len);
  if (new_socket < 0) {
    debugLog(NGB_ERROR, "TcpdAgent::tcpListen fail to accept message");
    return false;
  }
  close(socket_);
  socket_ = new_socket;

  registerReceiver(socket_);
  
  startReceiver();
  
  debugLog(NGB_TCP_AGENT, "TcpdAgent::tcpListen exit");
  return true;
}

int TcpdAgent::sendMsg(char *msg, int len)
{
  int version = 0;
  tcp_msg_hdr_t hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ver = htons(version);
  hdr.data_len = htonl(sizeof(tcp_msg_hdr_t) + len);
  int result = 0;
  result = writeData(socket_, (char*)&hdr, sizeof(hdr));
  if (result != sizeof(hdr)) {
    debugLog(NGB_ERROR, "TcpAgent::sendMg fail to send header");
    return false;
  }
  result = writeData(socket_, msg, len);
  if (result != len) {
    debugLog(NGB_ERROR, "TcpAgent::snedMsg fail to send body");
    return false;
  }
  return result;
}

int TcpdAgent::writeData(int fd, char* buf, int size)
{
  int offset = 0;
  int len_send = 0;
  int left = size;
  while (left > 0) {
    len_send = write(socket_, buf + offset, left);
    if (len_send < 0) {
      debugLog(NGB_ERROR, "TcpAgent::writeData fail to send message");
      return false;
    }
    left -= len_send;
    offset += len_send;
  }
  return offset;
}

int TcpdAgent::readData(char *buf, int size)
{
  debugLog(NGB_TCP_AGENT,
           "TcpAgent::readData need read %d byte data", size);
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
      debugLog(NGB_ERROR, "TcpAgent::readData fail to select");
      break;
    } else if (ret == 0) {
      // timeout
      continue;
    }
    len_read = read(socket_, buf + offset, left);
    debugLog(NGB_TCP_AGENT,
             "TcpAgent::readData read %d byte this time", len_read);
    if (len_read < 0) {
      debugLog(NGB_ERROR, "TcpAgent::readData fail to read");
      continue;
    }
    
    if (len_read == 0) {
      debugLog(NGB_ERROR,
               "TcpAgent::readData read zero data, connection may be closed");
      sleep(3);
      continue;
    }
    offset += len_read;
    left -= len_read;
  }
  debugLog(NGB_TCP_AGENT, "TcpAgent::readData %d byte read", offset);
  return offset;
}

int TcpdAgent::getQueueSize()
{
  return recMsgQueue_.size();
}

Message* TcpdAgent::receive()
{
  if (!recMsgQueue_.empty()) {
    Message* msg;
    msg = recMsgQueue_.front();
    recMsgQueue_.pop();
    return msg;
  }
  return (Message *)-1;
}

bool TcpdAgent::startReceiver()
{
  pthread_t thread;
  pthread_attr_t thread_attr;

  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  
  Thread2Ptr t = &TcpdAgent::receiverThreadFunc;
  PthreadPtr p = *(PthreadPtr*)&t;
  pthread_create(&thread, &thread_attr, p, this);

  return 0;
}

bool TcpdAgent::registerReceiver(int fd)
{
  return true;
}

bool TcpdAgent::getMsgWithTcpHeader(Message *msg)
{
  debugLog(NGB_TCP_AGENT, "TcpAgent::getMsgWithTcpHeader enter");
  tcp_msg_hdr_t hdr;
  int len =0;
  
  len = readData((char*)&hdr, sizeof(hdr));
  if (len != sizeof(hdr)) {
    debugLog(NGB_ERROR,
             "TcpAgent::receiverThreadFunc fatal network error");
    return false;
  }
  int ver = ntohs(hdr.ver);
  int payload_size = 0;
  payload_size = ntohl(hdr.data_len) - len;
  if (payload_size <= 0) {
    debugLog(NGB_ERROR,
             "TcpAgent::receiverThreadFunc invalid payload size");
    return false;
  }

  int got = readData(msg->getRawPtr(), payload_size);
  if (got != payload_size) {
    debugLog(NGB_ERROR,
             "TcpAgent::receiverThreadFunc fatal network error");
    return (void *)false;
  }

  debugLog(NGB_TCP_AGENT, "TcpAgent::getMsgWithTcpHeader exit");
  return true;
}

bool TcpdAgent::getMsgWithoutTcpHeader(Message *msg)
{
  debugLog(NGB_TCP_AGENT, "TcpAgent::getMsgWithoutTcpHeader enter");

  int len = readData( msg->getRawPtr(), sizeof(msg->hdr_));
  if (len != sizeof(dfs_msg_header)) {
    debugLog(NGB_ERROR,
             "TcpAgent::receiverThreadFunc fatal network error");
    return (void *)false;
  }
  
  msg->parseHdrRawToApp(msg->getRawPtr());
  msg->createAvpsWithNameAndType();
  
  int payload_size = 0;
  payload_size = msg->getBodySize();
  int got = readData(msg->getRawPtr() + sizeof(msg->hdr_),
                     payload_size);
  if (got != payload_size) {
    debugLog(NGB_ERROR,
             "TcpAgent::receiverThreadFunc fatal network error");
    return (void *)false;
  }
  
  debugLog(NGB_TCP_AGENT, "TcpAgent::getMsgWithoutTcpHeader exit");
  return true;
}

void* TcpdAgent::receiverThreadFunc()
{
  debugLog(NGB_TCP_AGENT, "TcpAgent::receiverThreadFunc enter");
  // Message *msg = new Message(ANSWER);
  Message *msg = new Message(REQUEST);
  while (1) {
    debugLog(NGB_TCP_AGENT,
             "TcpAgent::receiverThreadFunc going to get header");
    bool ret = false;
    if (ConfigManager::isTcpHeaderInResponse()) {
      ret = getMsgWithTcpHeader(msg);
    } else {
      ret = getMsgWithoutTcpHeader(msg);
    }

    if (!ret) {
      debugLog(NGB_ERROR,
               "TcpAgent::receiverThreadFunc fail to get message");
      return (void *)false;
    }
    
    recMsgQueue_.push(msg);
    debugLog(NGB_TCP_AGENT,
             "TcpAgent::receiverThreadFunc %d message in queue",
             recMsgQueue_.size());
  }
}

