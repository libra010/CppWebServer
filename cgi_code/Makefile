CXX = g++
CXXFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = ../resources_cgi/auth.cgi

SRCS = ../cgi_code/auth.cpp

all: 
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
