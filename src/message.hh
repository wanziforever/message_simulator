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

#ifndef NGB_MESSAGE_H__
#define NGB_MESSAGE_H__

#include <netinet/in.h>
#include "utils.hh"
#include "container.hh"

// temprarily use for test
#define DFS_INPUT_CHUNK 11
#define MAX_PATH_SIZE 256
#define PRIVATE_SIZE 128
#define MD5_CHECKSUM_LENGTH 16
#define MAX_MESSAGE_SIZE 2048

struct dfs_input_chunk {
  char file_path[MAX_PATH_SIZE];
  int idx;
  int padding;
  char priv_data[PRIVATE_SIZE];
  int priv_size;
  int size;
  unsigned long int handle;
  unsigned char checksum[MD5_CHECKSUM_LENGTH];
  int dst_ip[8];
  int dst_cnt;
  int reserved[121];
};

struct dfs_msg
{
  short int ver;
  short int op;
  short int reply;
  short int errcode;
  struct dfs_input_chunk inchk;
};

struct udp_header {
  unsigned char ver;
  unsigned char padding[7];
  unsigned long int seq;
  unsigned char reserved[8];
};


typedef struct mhandle {
  struct sockaddr_in src;
  struct udp_header hdr;
  int fd;
} mhandle;

class Message
{
public:
  // constructor used to read human readable format from file
  // normally used for parsing app to raw format.
  Message(std::string path) : path_(path) {}
  // constructor normally used for parsing raw to app
  Message() : path_("") {}
  bool init();
  int parseRawToApp(char *output);
  int parseAppToRaw(char *output);
  void print();
  bool readMsgFile();

private:
  int parseHdrRawToApp(char *output);
  int parseHdrAppToRaw(char *output);
  int parseBodyRawToApp(char *output);
  int parseBodyAppToRaw(char *output);
  bool fillAvpsWithTypes();
  bool createAvpsWithNameAndType();
  std::string command_;
  udp_header hdr_;
  // Container requestContainer_;
  Container avpContainer_;
  char raw_[MAX_MESSAGE_SIZE];
  std::string path_;
};
  
#endif
