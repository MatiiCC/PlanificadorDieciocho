CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = planificador
SRCS = planificador.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
