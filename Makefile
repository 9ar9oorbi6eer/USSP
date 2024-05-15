# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# The target executable
TARGET = USPassignment

# Source files
SRCS = USPassignment.c functions.c processes.c

# Object files
OBJS = $(SRCS:.c=.o)

# Header files
HEADERS = functions.h processes.h

# Default rule
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .c files into .o files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build files
clean:
	rm -f $(OBJS) $(TARGET)

# Rebuild the project from scratch
rebuild: clean all
