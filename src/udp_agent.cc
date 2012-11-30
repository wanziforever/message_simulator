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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "udp_agent.hh"
#include "log.hh"
#include "message.hh"

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

  // start receiver thread
}

#define NGB_CREATE_MSG_HEADER(handle, ip, port) \
  bzero(&handle, sizeof(handle));               \
  handle.src.sin_family = AF_INET;              \
  handle.src.sin_addr.s_addr = inet_addr(ip);   \
  handle.src.sin_port = htons(port);            \
  handle.hdr.seq = 12;                          \
  handle.hdr.ver = 0;

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
  iov[0].iov_base = &handle.hdr;
  iov[0].iov_len = sizeof(handle.hdr);
  iov[1].iov_base = msg;
  iov[1].iov_len = len;

  struct msghdr hdr = {0};
  hdr.msg_iov = iov;
  hdr.msg_iovlen = sizeof(iov) / sizeof(iov[0]);
  hdr.msg_name = &handle.src;
  hdr.msg_namelen = sizeof(handle.src);
  int r = sendmsg(socket_, &hdr, 0/*flags*/);
  if (r == -1) {
    debugLog(NGB_UDP_AGENT,
             "UdpAgent::sendMsg fail to send message, errno(%d)", errno);
    return false;
  }
  debugLog(NGB_UDP_AGENT, "%d byte was send", r);

  return true;
}
