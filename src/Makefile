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
TARGET_D = $(MAIN)d
TARGET_PERF_U = ngbPerfudp
TARGET_PERF_T = ngbPerftcp

UT_TARGET = ngbSim_unittest
build_root=$(shell pwd)/..
INCLUDE_DIR = -I. -I$(build_root)/inc
LIB_DIR = -L$(build_root)/lib
LIBS    = -lxerces-c -lpthread -lcurses
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
	tcpd_agent.cc \
	container.cc \
	message.cc \
	avp_type.cc \
	perf_config_manager.cc 


PERF_SRC = view.cc \
	load.cc

OBJ_U = $(addsuffix _u.o, $(basename $(SRC)))
OBJ_T = $(addsuffix _t.o, $(basename $(SRC)))
OBJ_D = $(addsuffix _d.o, $(basename $(SRC)))
OBJ_PERF_U = $(addsuffix _u.o, $(basename $(PERF_SRC)))
OBJ_PERF_T = $(addsuffix _t.o, $(basename $(PERF_SRC)))

# also can use
# OBJ = $(subst .cc, .o, $(SRC))

build_bin = $(build_root)/bin

#
#all: $(build_bin)/$(TARGET_U) \
#	$(build_bin)/$(TARGET_T) \
#	$(build_bin)/$(TARGET_D) \
#	$(build_bin)/$(TARGET_PERF_U) \
#	$(build_bin)/$(TARGET_PERF_T)

all: $(build_bin)/$(TARGET_PERF_U)

#
# compile the main function
$(TARGET_U).o:$(MAIN).cc
	$(CXX) -DUDP $(INCLUDE_DIR) -c $< -o $@

$(TARGET_T).o:$(MAIN).cc
	$(CXX) -DTCP $(INCLUDE_DIR) -c $< -o $@

$(TARGET_D).o:$(MAIN).cc
	$(CXX) -DTCPD $(INCLUDE_DIR) -c $< -o $@

$(TARGET_PERF_U).o:ngbPerf.cc
	$(CXX) -DUDP $(INCLUDE_DIR) -c $< -o $@

$(TARGET_PERF_T).o:ngbPerf.cc
	$(CXX) -DTCP $(INCLUDE_DIR) -c $< -o $@

$(OBJ_U):%_u.o:%.cc
	$(CXX) -DUDP $(INCLUDE_DIR) -c $< -o $@

$(OBJ_T):%_t.o:%.cc
	$(CXX) -DTCP $(INCLUDE_DIR) -c $< -o $@

$(OBJ_D):%_d.o:%.cc
	$(CXX) -DTCPD $(INCLUDE_DIR) -c $< -o $@

$(OBJ_PERF_U):%_u.o:%.cc
	$(CXX) -DUDP $(INCLUDE_DIR) -c $< -o $@

$(OBJ_PERF_T):%_t.o:%.cc
	$(CXX) -DTCP $(INCLUDE_DIR) -c $< -o $@

#$(UT_OBJ):%o:%cc
#	$(CXX) $(INCLUDE_DIR) -c $< -o $@

$(build_bin)/$(TARGET_U): $(TARGET_U).o $(OBJ_U)
	@echo "build UDP binary..."
	$(CXX) $^ -o $@ $(LIB_DIR) $(LIBS)

$(build_bin)/$(TARGET_T): $(TARGET_T).o $(OBJ_T)
	@echo "build TCP binary..."
	$(CXX) $^ -o $@ $(LIB_DIR) $(LIBS)

$(build_bin)/$(TARGET_D): $(TARGET_D).o $(OBJ_D)
	@echo "build TCP binary..."
	$(CXX) $^ -o $@ $(LIB_DIR) $(LIBS)

$(build_bin)/$(TARGET_PERF_U): $(TARGET_PERF_U).o $(OBJ_PERF_U) $(OBJ_U)
	@echo "build UDP PERFORMANCE binary..."
	@echo $(OBJ_PERF_U)
	$(CXX) $^ -o $@ $(LIB_DIR) $(LIBS)

$(build_bin)/$(TARGET_PERF_T): $(TARGET_PERF_T).o $(OBJ_PERF_T) $(OBJ_T)
	@echo "build TCP PERFORMANCE binary..."
	$(CXX) $^ -o $@ $(LIB_DIR) $(LIBS)

clean:
	rm -rf *.o
