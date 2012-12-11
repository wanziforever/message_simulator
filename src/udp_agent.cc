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
#include "udp_agent.hh"
#include "log.hh"

#define MAX_EVENTS 10

typedef void* (UdpAgent::*Thread2Ptr)(void);
typedef void* (*PthreadPtr)(void*);

UdpAgent::~UdpAgent()
{
  if (socket_)
    close(socket_);
}

// initialize the UDP agent socket, bind the local port, and start
// a receiver thread to receve message, the receiver thread will
// put the received message to the message queue
bool UdpAgent::init(int localPort)
{
  debugLog(NGB_UDP_AGENT,
           "UdpAgent::init enter with local port(%d)", localPort);
  if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    debugLog(NGB_UDP_AGENT, "UdpAgent::init fail to create UDP socket");
    return false;
  }
  // bind local port
  struct sockaddr_in src_addr;
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  src_addr.sin_port = localPort;
  if (bind(socket_, (struct sockaddr *)&src_addr,
           sizeof(src_addr)) == -1) {
    debugLog(NGB_UDP_AGENT, "UdpAgent::init fail to bind local");
    return false;
  }
  // start 
  registerReceiver(socket_);
  // start receiver thread
  startReceiver();
}

#define NGB_CREATE_MSG_HEADER(handle, ip, port) \
  bzero(&handle, sizeof(handle));               \
  handle.src.sin_family = AF_INET;              \
  handle.src.sin_addr.s_addr = inet_addr(ip);   \
  handle.src.sin_port = htons(port);            \
  handle.hdr.seq = 12;                          \
  handle.hdr.ver = 0;

#define INIT_IOVEC(hdr, iov, buf, buf_len) \
  iov[0].iov_base = &hdr;                  \
  iov[0].iov_len = sizeof(hdr);     \
  iov[1].iov_base = buf;                   \
  iov[1].iov_len = buf_len;

#define INIT_IOVEC_MSG(hdr, iov, src)             \
  hdr.msg_iov = iov;                              \
  hdr.msg_iovlen = sizeof(iov) / sizeof(iov[0]);  \
  hdr.msg_name = &src;                            \
  hdr.msg_namelen = sizeof(src);

// send UDP message out by iovec implementation, there is no security
// consideration currently, so the version in the message was hard code
// to 0, and also the sequence in the message was hard code to 12
bool UdpAgent::sendMsg(const char *ip, int port, char *msg, int len)
{
  debugLog(NGB_UDP_AGENT,
           "UdpAgent::sendMsg enter with ip(%s), port(%d), len(%d)",
           ip, port, len);

  mhandle handle;
  NGB_CREATE_MSG_HEADER(handle, ip, port);
  struct iovec iov[2];
  INIT_IOVEC(handle.hdr, iov, msg, len);
  struct msghdr hdr = {0};
  INIT_IOVEC_MSG(hdr, iov, handle.src);

  int r = sendmsg(socket_, &hdr, 0/*flags*/);
  if (r == -1) {
    debugLog(NGB_UDP_AGENT,
             "UdpAgent::sendMsg fail to send message, errno(%d)", errno);
    return false;
  }
  debugLog(NGB_UDP_AGENT, "%d byte was send", r);

  return true;
}

// there is no consideration about the performance here, whole received message
// will be put in the queue, there is also no race condition consideration later
void* UdpAgent::receiverThreadFunc()
{
  debugLog(NGB_UDP_AGENT, "UdpAgent::receiverThreadFunc enter");
  Message msg;
  struct iovec iov[2];
  struct in_addr src;
  udp_header udpheader;
  INIT_IOVEC(udpheader, iov, msg.getRawPtr(), COMMON_MSG_SIZE);
  struct msghdr hdr = {0};
  INIT_IOVEC_MSG(hdr, iov, src);
  while(1) {
    int r = recvmsg(socket_, &hdr, 0 /*flags*/);
    // need to convert the seq after receive the message?
    if (r < 1) {
      debugLog(NGB_UDP_AGENT,
               "UdpAgent::receiverThreadFunc received invalid message");
      continue;
    }
    // put the message to message queue
    msg.setUdpHeader(udpheader);
    recMsgQueue_.push(msg);
  }
}

int UdpAgent::getQueueSize()
{
  return recMsgQueue_.size();
}


bool UdpAgent::startReceiver()
{
  pthread_t thread;
  pthread_attr_t thread_attr;

  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  
  Thread2Ptr t = &UdpAgent::receiverThreadFunc;
  PthreadPtr p = *(PthreadPtr*)&t;
  pthread_create(&thread, &thread_attr, p, this);

  return 0;
}

bool UdpAgent::registerReceiver(int fd)
{
  // pass
  return true;
}
