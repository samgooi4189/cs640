# Makefile
# Requires that the object definitions be in wl.h and wl.cpp

CXX =           g++
PURXX = purify
CXXFLAGS =     -std=c++0x -O2 -g -Wall

# During debugging you may want to used the compiler flags listed below
#CXXFLAGS =      -g -Wall

all: wl

wl: wl.cpp 
	$(CXX) $(CXXFLAGS) wl.cpp -o $@

wl.pure : wl.cpp 
	$(PURXX) $(CXX) $(CXXFLAGS) wl.cpp -o $@

clean:
	rm -f core *.o wl wl.pure

