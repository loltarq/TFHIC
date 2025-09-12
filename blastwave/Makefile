# ---------------------------------------
# ROOT-config automatically gets the flags!
# ---------------------------------------
ROOTCFLAGS    := $(shell root-config --cflags)
ROOTLIBS      := $(shell root-config --libs)
ROOTGLIBS     := $(shell root-config --glibs)

# Compiler & flags
CXX           := g++

# --- Apps (minimal flags) ---
BINDIR ?= bin
APP_SRCS := apps/blastwave_thermal.cpp
APPS := $(APP_SRCS:apps/%.cpp=$(BINDIR)/%)

# Sources & output library name
SRCS          := src/hadron_list.cpp src/io_utils.cpp src/blastwave_utils.cpp src/io_thermal.cpp
OBJS          := $(SRCS:.cpp=.o)

LIBNAME       := libTFHIC.so
INCLUDES      := -Iinclude

CXXFLAGS      := -O2 -Wall -fPIC -std=c++17 $(ROOTCFLAGS) $(INCLUDES)

# ---------------------------------------
# Default rule: build the shared library
# ---------------------------------------
all: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(CXX) -shared -o $@ $(OBJS) $(ROOTLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR)/%: apps/%.cpp $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -Iinclude $< $(OBJS) -o $@ $(ROOTLIBS)
	
all: $(LIBNAME) $(APPS)

# ---------------------------------------
# Clean rule
# ---------------------------------------
.PHONY: clean
clean:
	rm -f $(OBJS) $(LIBNAME)
	rm -f $(BINDIR)/blastwave_thermal   # <-- remove the app
