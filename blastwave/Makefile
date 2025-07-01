# ---------------------------------------
# ROOT-config automatically gets the flags!
# ---------------------------------------
ROOTCFLAGS    := $(shell root-config --cflags)
ROOTLIBS      := $(shell root-config --libs)
ROOTGLIBS     := $(shell root-config --glibs)

# Compiler & flags
CXX           := g++


# Sources & output library name
SRCS          := src/hadron_list.cpp src/io_utils.cpp src/blastwave_utils.cpp
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

# ---------------------------------------
# Clean rule
# ---------------------------------------
clean:
	rm -f $(OBJS) $(LIBNAME)