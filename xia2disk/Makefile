#nice and simple make file using typical implicit rules

#include ../makepixie.inc
include makepixie.inc

#vpath %.cpp src
#vpath %.c src

COMMONOBJS  = $(INTERFACELIB) $(INTERFACE)/Utility.o
POLLOBJS    = poll.o sendbuf.o spkt_io_udp.o StatsHandler.o
POLLCJGOBJS = xia2disk.o StatsHandler.o #xia2try1.o sendbuf.o spkt_io_udp.o
PULSEROBJS  = pulser.o sendbuf.o spkt_io_udp.o
COMMOBJS    = commtest.o sendbuf.o spkt_io_udp.o
OBJS        = $(POLLOBJS) $(COMMOBJS) $(PULSEROBJS) 
TARGETS     = xia2disk  #xia2try #poll-cjg poll-map poll-scan xia2disk  commtest pulser 

# make things copasetic with c++
LINK.o      = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

.phony: clean all
all: $(TARGETS)

xia2disk: $(POLLCJGOBJS) $(COMMONOBJS) xia2disk

#poll: $(POLLOBJS) $(COMMONOBJS)

commtest: $(COMMOBJS)

pulser: $(PULSEROBJS) $(COMMONOBJS)

#poll-cjg: $(POLLCJGOBJS) $(COMMONOBJS) poll-cjg

#poll-map: $(POLLCJGOBJS) $(COMMONOBJS) poll-map

#poll-scan: $(POLLCJGOBJS) $(COMMONOBJS) poll-scan

#xia2try1: $(POLLCJGOBJS) $(COMMONOBJS) xia2try1

clean: 
	@echo "Cleaning up..."
	@$(RM) $(OBJS) $(TARGETS) *~

