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
#include "udpd_agent.hh"
#include "log.hh"

#define MAX_EVENTS 10

static pthread_mutex_t udp_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
#define UDP_QUEUE_LOCK()  pthread_mutex_lock(&udp_queue_mutex)
#define UDP_QUEUE_UNLOCK()  pthread_mutex_unlock(&udp_queue_mutex)

typedef void* (UdpdAgent::*Thread2Ptr)(void);
typedef void* (*PthreadPtr)(void*);

UdpdAgent::~UdpdAgent()
{
  if (socket_)
    close(socket_);
}

// initialize the UDP agent socket, bind the local port, and start
// a receiver thread to receve message, the receiver thread will
// put the received message to the message queue
bool UdpdAgent::init(int localPort)
{
  debugLog(NGB_UDP_AGENT,
             "UdpAgent::init error counter is %ld",
             errorCounter_);
  debugLog(NGB_UDP_AGENT,
           "UdpAgent::init enter with local port(%d)", localPort);
  if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    debugLog(NGB_ERROR, "UdpAgent::init fail to create UDP socket");
    return false;
  }
  // bind local port
  struct sockaddr_in src_addr;
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  src_addr.sin_family = AF_INET;
  src_addr.sin_port = htons(localPort);
  if (bind(socket_, (struct sockaddr *)&src_addr,
           sizeof(src_addr)) == -1) {
      debugLog(NGB_ERROR,
               "UdpAgent::init fail to bind local, errno is %d", errno);
    return false;
  }
  // start 
  registerReceiver(socket_);
  // start receiver thread
  startReceiver();
}

#define NGB_CREATE_MSG_HEADER(handle, dest)               \
  bzero(&handle, sizeof(handle));                             \
  handle.src = dest;                            \
  handle.hdr.seq = 12;                                        \
  handle.hdr.ver = 0; 

#define INIT_IOVEC(hdr, iov, buf, buf_len) \
  iov[0].iov_base = &hdr;                  \
  iov[0].iov_len = sizeof(hdr);            \
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
bool UdpdAgent::sendMsg(char *msg, int len)
{
  debugLog(NGB_UDP_AGENT,
           "UdpAgent::sendMsg enter with lenn(%d)", len);

  if (!destinationSet_)
    return false;
  char fromIP[20] = {0};
  inet_ntop(AF_INET, &(src_.sin_addr), fromIP, sizeof(fromIP));
  int port = 0;
  port = ntohs(src_.sin_port);
  debugLog(NGB_UDP_AGENT,
           "UdpAgent::sendMsg to ip(%s), port(%d)", fromIP, port);
  mhandle handle;
  NGB_CREATE_MSG_HEADER(handle, src_);
  struct iovec iov[2];
  //handle.hdr.seq = Utils::getTimeOfDayinUsec();
  handle.hdr.seq = seq_;

  INIT_IOVEC(handle.hdr, iov, msg, len);

  debugLog(NGB_UDP_AGENT,
           "UdpAgent::sendMsg the hdr size is %d", sizeof(handle.hdr));
  struct msghdr hdr = {0};
  INIT_IOVEC_MSG(hdr, iov, handle.src);

  int r = sendmsg(socket_, &hdr, 0/*flags*/);
  if (r == -1) {
    debugLog(NGB_ERROR,
             "UdpAgent::sendMsg fail to send message, errno(%d)", errno);
    return false;
  }
  debugLog(NGB_UDP_AGENT, "%d byte was send", r);

  return true;
}

// there is no consideration about the performance here, whole received
// message will be put in the queue, there is also no race condition
// consideration later
void* UdpdAgent::receiverThreadFunc()
{
  debugLog(NGB_UDP_AGENT, "UdpAgent::receiverThreadFunc enter");
  while (1) {
    Message *msg = new Message(REQUEST);
    struct iovec iov[2];
    struct sockaddr_in src;
    udp_header udpheader;
    INIT_IOVEC(udpheader, iov, msg->getRawPtr(), COMMON_MSG_SIZE);
    struct msghdr hdr = {0};
    INIT_IOVEC_MSG(hdr, iov, src);
    debugLog(NGB_UDP_AGENT, "UdpAgent::receiverThreadFunc queue size is %d", recMsgQueue_.size());
    int r = recvmsg(socket_, &hdr, 0 /*flags*/);
    seq_ = udpheader.seq;
    src_ = src;
    destinationSet_ = true;
    debugLog(NGB_UDP_AGENT,
             "UdpAgent::receiverThreadFunc got one message");
    // need to convert the seq after receive the message?
    if (r < 1) {
      debugLog(NGB_ERROR,
               "UdpAgent::receiverThreadFunc received invalid message");
      continue;
    }

    debugLog(NGB_UDP_AGENT,
             "UdpAgent::receiverThreadFunc msg queue size is %u",
             recMsgQueue_.size());
    // put the message to message queue
    UDP_QUEUE_LOCK();
    recMsgQueue_.push(msg);
    debugLog(NGB_UDP_AGENT, "UdpAgent::receiveThreadFunc msg is %u", msg);
    totalCounter_++;
    UDP_QUEUE_UNLOCK();

    debugLog(NGB_UDP_AGENT,
             "UdpAgent::receiverThreadFunc msg queue size is %ld",
             errorCounter_);
    
    // check the message error flag
    struct dfs_msg_header *dfs_hdr = (struct dfs_msg_header*)msg->getRawPtr();
    if (dfs_hdr->reply == 1) {
      errorCounter_++;
    }
  }
}

int UdpdAgent::getQueueSize()
{
  return recMsgQueue_.size();
}


bool UdpdAgent::startReceiver()
{
  pthread_t thread;
  pthread_attr_t thread_attr;

  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  
  Thread2Ptr t = &UdpdAgent::receiverThreadFunc;
  PthreadPtr p = *(PthreadPtr*)&t;
  pthread_create(&thread, &thread_attr, p, this);

  return 0;
}

bool UdpdAgent::registerReceiver(int fd)
{
  // pass
  return true;
}

// a nsync function call
Message* UdpdAgent::receive()
{
  UDP_QUEUE_LOCK();
  debugLog(NGB_UDP_AGENT, "UdpAgent::receive queue size is %d", recMsgQueue_.size());
  if (!recMsgQueue_.empty()) {
    Message *msg;
    msg = recMsgQueue_.front();
    recMsgQueue_.pop();
    UDP_QUEUE_UNLOCK();
    return msg;
  }
  UDP_QUEUE_UNLOCK();
  return (Message *) -1;
}

void UdpdAgent::clearQueue()
{
  UDP_QUEUE_LOCK();
  std::queue<Message*> empty;
  std::swap(recMsgQueue_, empty);
  UDP_QUEUE_UNLOCK();
}
