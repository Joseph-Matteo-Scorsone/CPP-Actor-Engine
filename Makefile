# Compiler to use
CC = g++

# Warning flags - enables most common warnings
WARNINGS = -Wall -Wextra -pedantic -Wshadow -Wformat=2 -Wfloat-equal \
           -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond \
           -Wcast-qual -Wcast-align

# Standard flags
CFLAGS = -std=c++23 $(WARNINGS)

# Name of the output executable
EXECUTABLE = main.exe

# Source files
SRC = main.cpp

# Object files
OBJ = $(SRC:.cpp=.o)

# Default target (compiles the program)
all: $(EXECUTABLE)

# Compile the program
$(EXECUTABLE): $(OBJ)
	$(CC) $(OBJ) -o $(EXECUTABLE)

# Compile .cpp files to .o files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run: $(EXECUTABLE)
	./$(EXECUTABLE)

# Clean build files
clean:
	rm -f $(OBJ) $(EXECUTABLE)
