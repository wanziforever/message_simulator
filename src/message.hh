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
#include <map>
#include "utils.hh"
#include "container.hh"
#include "dictionary_manager.hh"

// temprarily use for test
#define DFS_INPUT_CHUNK 11
#define MAX_PATH_SIZE 256
#define PRIVATE_SIZE 128
#define MD5_CHECKSUM_LENGTH 16
#define COMMON_MSG_SIZE 1500

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

struct dfs_msg_header
{
  short int ver;
  short int op;
  short int reply;
  short int errcode;
};

typedef struct udp_header {
  unsigned char ver;
  unsigned char padding[7];
  unsigned long long seq;
  unsigned char reserved[8];
} udp_header;


typedef struct mhandle {
  struct sockaddr_in src;
  struct udp_header hdr;
  int fd;
} mhandle;

class GroupRawEntry;
class RawEntryContainer;
class RawEntry
{
public:
  RawEntry() : name_(""), value_(""), parent_(0), deepth_(0) {}
  ~RawEntry() {}
  void setName(std::string name) { name_ = name; }
  void setValue(std::string value) { value_ = value; }
  void setDeepth(int deepth) { deepth_ = deepth; }
  void setParent(GroupRawEntry* pContainer) {parent_ = pContainer; }
  std::string getName() { return name_; }
  std::string getValue() { return value_; }
  int getDeepth() { return deepth_; }
  GroupRawEntry* getParent() { return parent_; }
  std::string getSignature() { return signature_; }
  virtual std::string toString(int numOfIndent = 0);
  virtual std::string getDisplayData();
  std::string generateSignature();
  
protected:
  std::string name_;
  std::string value_;
  GroupRawEntry* parent_;
  int deepth_;
  std::string signature_;
};

class RawEntryContainer
{
public:
  RawEntryContainer() {}
  ~RawEntryContainer() {}
  //virtual RawEntry* appendEntry(std::string, std::string value);
  //virtual RawEntryContainer* appendContainer(std::string name);
  virtual std::string toString(int numOfIndent = 0);
  RawEntry* fetchEntryBySignature(std::string signature);
  bool setSignature(std::string sign, RawEntry *rawEntry);
protected:
  std::vector<RawEntry*> childEntries_;
  std::map<std::string, RawEntry*> signatureMapping_;
};

class GroupRawEntry : public RawEntry
{
public:
  GroupRawEntry() {}
  ~GroupRawEntry() {}
  RawEntry* appendEntry(std::string, std::string value);
  GroupRawEntry* appendContainer(std::string name);
  // get string print to debug log
  std::string toString(int numOfIndent = 0);
  // get string print to display
  std::string getDisplayData();
  bool isRoot() { return isRootGroup_; }
  void setRoot(bool yesORno) { isRootGroup_ = yesORno; }
protected:
  std::vector<RawEntry*> childEntries_;
  bool isRootGroup_;
};

class Message
{
  friend class TcpAgent;
  friend class TcpdAgent;
public:
  // constructor used to read human readable format from file
  // normally used for parsing app to raw format.
  Message(std::string path, short direction = REQUEST) :
    path_(path), commandCode_(0),command_(""), errorCode_(0),
    direction_(direction), bodySize_(0) {
    rootGroupRawEntry_.setRoot(true);
  }
  // constructor normally used for parsing raw to app
  Message(short direction = REQUEST) :
    path_(""), commandCode_(0), command_(""), errorCode_(0),
    direction_(direction), bodySize_(0) {
    rootGroupRawEntry_.setRoot(true);
  }
  bool init();
  int parseRawToApp(char *output);
  int parseAppToRaw(char *output);
  void printRawEntry();
  void printDebug();
  std::string getDisplayData();
  bool readMsgFile();
  char* getRawPtr() { return (char*)raw_; }
  char* getHeader() { return (char*)&hdr_; }
  int getBodySize() { return bodySize_; }
  int parseHdrRawToApp(char *output);
  int parseHdrAppToRaw(char *output);
  int parseBodyRawToApp(char *output);
  int parseBodyAppToRaw(char *output);
private:
  bool generateMessageAvps();
  bool fillAvpsWithTypes();
  bool createAvpsWithNameAndType();
  bool parseCommand(std::string entry);
  bool convertToRaw();

  // member variables
  std::string command_;
  short int commandCode_;
  // udp_header hdr_;
  struct dfs_msg_header hdr_;
  GroupRawEntry rootGroupRawEntry_;
  MessageEntryContainer avpContainer_;
  char raw_[COMMON_MSG_SIZE];
  std::string path_;
  int errorCode_;
  short direction_; // 1:request, 0:answer
  int bodySize_;
};
  
#endif
