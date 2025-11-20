CXX = g++
CXXFLAGS = -I. -Wall -std=c++11

TARGET = vm_test
SRCS = test_runner.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
