CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = program
OBJS = ../code/*.cpp ../code/buffer/*.cpp ../code/http/*.cpp ../code/log/*.cpp ../code/timer/*.cpp ../code/server/*.cpp
all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)