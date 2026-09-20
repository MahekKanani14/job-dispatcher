CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -pthread

MASTER_SRCS = src/master.cpp src/job_queue.cpp src/scheduler.cpp src/logger.cpp

all: build/master build/worker

build/master: $(MASTER_SRCS)
	@mkdir -p build logs
	$(CXX) $(CXXFLAGS) -o $@ $(MASTER_SRCS)

build/worker: src/worker.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ src/worker.cpp

clean:
	rm -f build/master build/worker
	rm -f logs/results.csv

.PHONY: all clean
