# Makefile to build the efficiency_extrap CLI (requires ROOT)
CXX      := g++
CXXFLAGS := -O2 -std=c++17 $(shell root-config --cflags)

# Include paths:
#  - ./include           (bw_adapter.h, eff_extrap.h)
#  - ../common           (hadron_catalog.h/.cpp)
#  - ../blastwave/include (third_party/json.hpp etc)
INC := -Iinclude -I. -I../common -I../blastwave/include

LDFLAGS  := $(shell root-config --libs)

APP := bin/efficiency_extrap

# Add the catalog implementation here:
SRC := apps/efficiency_extrap.cpp \
       ../common/hadron_catalog.cpp

all: $(APP)

$(APP): $(SRC)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(APP)
