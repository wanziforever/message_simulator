# Message simulator: tool for simulate internal message for
# NGB system
#
# Copyright (C) 2012, Hisense China R&D, Inc.
#
# the software is free to read and modification, but is restrict
# for any distribution, please contact the owner if have any
# questions.
#
# @Author wangliang8@hisense.com (denny)
# Version: 0.1 2012/09/29
#
# all the object(*.o) file will be generated with source code in
# the same directory for the simpler Makefile orgnization, the
# make file will also build the unit test binary file, please add
# unite test source code to UT_SRC variable, gtest was selected
# as the unit test framework tool.
#
PLATFORM = LINUX
CXX = g++
CC = gcc

MAIN = ngbSim
TARGET_U  = $(MAIN)udp
TARGET_T = $(MAIN)tcp

UT_TARGET = ngbSim_unittest
build_root=$(shell pwd)/..
INCLUDE_DIR = -I. -I$(build_root)/inc
LIB_DIR = -L$(build_root)/lib
LIBS    = -lxerces-c -lpthread
# gtest library with main function for unit test
GTEST_LIB_MAIN_STATIC = $(build_root)/lib/gtest_main.a

# the source code list, file name should be suffixed with .cc
SRC = \
	dictionary_manager.cc \
	DOMTreeErrorReporter.cc \
	log.cc \
	log_mgr.cc \
	utils.cc \
	config_manager.cc \
	task.cc \
	run_item.cc \
	udp_agent.cc \
	tcp_agent.cc \
	container.cc \
	message.cc \
	avp_type.cc


OBJ_U = $(addsuffix _u.o, $(basename $(SRC)))
OBJ_T = $(addsuffix _t.o, $(basename $(SRC)))
# also can use
# OBJ = $(subst .cc, .o, $(SRC))

# unit test source code list
#UT_SRC = \
#	dictionaryManager_unittest.cc
#UT_OBJ = $(addsuffix .o, $(basename $(UT_SRC)))

build_bin = $(build_root)/bin

#
all: $(build_bin)/$(TARGET_U) \
	$(build_bin)/$(TARGET_T)
#	$(build_bin)/$(UT_TARGET)	
#
# compile the main function
$(TARGET_U).o:$(MAIN).cc
	$(CXX) -DUDP $(INCLUDE_DIR) -c $< -o $@
$(TARGET_T).o:$(MAIN).cc
	$(CXX) -DTCP $(INCLUDE_DIR) -c $< -o $@

$(OBJ_U):%_u.o:%.cc
	$(CXX) -DUDP $(INCLUDE_DIR) -c $< -o $@

$(OBJ_T):%_t.o:%.cc
	$(CXX) -DTCP $(INCLUDE_DIR) -c $< -o $@

#$(UT_OBJ):%o:%cc
#	$(CXX) $(INCLUDE_DIR) -c $< -o $@

$(build_bin)/$(TARGET_U): $(TARGET_U).o $(OBJ_U)
	@echo "build UDP binary..."
	$(CXX) $^ -o $@ $(LIB_DIR) $(LIBS)

$(build_bin)/$(TARGET_T): $(TARGET_T).o $(OBJ_T)
	@echo "build TCP binary..."
	$(CXX) $^ -o $@ $(LIB_DIR) $(LIBS)

#$(build_bin)/$(UT_TARGET): $(UT_OBJ) $(OBJ)
#	@echo "build Unit test binary..."
#	$(CXX) $^ -o $@ $(LIB_DIR) $(LIBS) $(GTEST_LIB_MAIN_STATIC)

clean:
	rm -rf *.o
