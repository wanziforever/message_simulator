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
//
// the unit test will use XML document as input, any document change require
// code change.

#include <gtest/gtest.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>

#include "dictionaryManager.hh"
#include "DOMTreeErrorReporter.hh"

#define DICTIONARY_MANAGER_INIT                 \
  DictionaryManager dictMgr;                    \
  dictMgr.init("dictionary.xml");

using namespace xercesc;

// currently the dictionaryxml should have exactly 2 command definitions
TEST(DictionaryManager,getNumOfCommand)
{
  DICTIONARY_MANAGER_INIT;
  EXPECT_EQ(2,dictMgr.getNumOfCommand());
}

TEST(DictionaryManager,getNumOfAvp)
{
  DICTIONARY_MANAGER_INIT;
  EXPECT_EQ(4,dictMgr.getNumOfAvp());
}

// TEST(DictionaryManager,getAllCommand)
// {
//   DICTIONARY_MANAGER_INIT;
//   int num = dictMgr.getNumOfCommand();
//   Command *commandList = new Command[num];
//   if (!commandList) {
//     std::cout << "memory allocation for comamnd list fail!" << std::endl;
//     return;
//   }
//   if (dictMgr.getAllCommands(commandList)) {
//     std::cout << "Fail to get all commands" << std::endl;
//     return;
//   }
// 
//   // make sure to have 2 commands in document
//   EXPECT_EQ("GET_CHUNK", commandList[0].getName());
//   EXPECT_EQ("LOAD_SEED", commandList[1].getName());
// }
