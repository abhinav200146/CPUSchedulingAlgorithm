# Define the compiler and compiler flags
CXX = g++
CXXFLAGS = -Wall -std=c++11

# Define the target executable
TARGET = myprogram

# Define the source and object files
SRCS = main.cpp parser.c
OBJS = $(SRCS:.cpp=.o)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to build object files from source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJS)
