CXX = g++
CXXFLAGS = -I. -Wall -std=c++11

TARGET = vm_test
RUNNER_TARGET = vm_runner
SRCS = test_runner.cpp
RUNNER_SRCS = vm_runner.cpp

all: $(TARGET) $(RUNNER_TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

$(RUNNER_TARGET): $(RUNNER_SRCS)
	$(CXX) $(CXXFLAGS) -o $(RUNNER_TARGET) $(RUNNER_SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(RUNNER_TARGET)
