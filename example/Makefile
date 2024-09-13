###########################################
# A Simple Makefile
###########################################

MARCH = $(subst _,-,$(shell arch))

CXXFLAGS = -O2 -Wall -Wno-char-subscripts -march=$(MARCH) -fpermissive -std=c++11
ifdef DEBUG
	CXXFLAGS += -g
endif

NAME = $(shell pwd|awk -F'/' '{print $$NF}')
EXENAME := $(NAME)
OUTDIR ?= .
INTDIR := $(OUTDIR)/objs
$(shell mkdir -p $(INTDIR))

INC_LOCAL = -I/usr/local/include
LIB_LOCAL = -L/usr/local/lib

INCLUDES := -I. -I../moss $(INC_LOCAL)
LINKLIBS := -L. -L$(OUTDIR) -lpthread -lmoss $(LIB_LOCAL)
LINKFLAGS = -Wl,-rpath=.

C_FILES := $(shell find . -name '*.c' ! -path "./test/*")
CPP_FILES := $(shell find . -name '*.cpp' ! -path "./test/*")
C_OBJECTS := $(subst /,@,$(patsubst ./%,%,$(C_FILES:.c=.o)))
CPP_OBJECTS := $(subst /,@,$(patsubst ./%,%,$(CPP_FILES:.cpp=.obj)))

OBJECTS := $(C_OBJECTS) $(CPP_OBJECTS)

.PHONY: all clean

all:$(EXENAME)

$(EXENAME): $(OBJECTS)
	$(CXX) $(LINKFLAGS) -o $(OUTDIR)/$@ $(LINKLIBS) $(foreach obj,$(OBJECTS),$(patsubst %,$(INTDIR)/%,$(obj)))
	chmod +x $(OUTDIR)/$@

%.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(subst @,/,$(patsubst %.o,%.c,$@)) -o $(INTDIR)/$@
%.obj:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(subst @,/,$(patsubst %.obj,%.cpp,$@)) -o $(INTDIR)/$@
	
clean:
	rm -f $(OUTDIR)/$(EXENAME)
	rm -rf $(INTDIR)
