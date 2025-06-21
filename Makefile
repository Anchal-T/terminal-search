# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
INCLUDES = -Iheaders -I/usr/include/libxml2
LIBS = -lcurl -lcjson -lncurses -lxml2

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/terminal_search

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "Build successful! Run with: ./$(TARGET) \"your search query\""

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)
	@echo "Clean complete!"

# Deep clean (remove directories too)
clean-all:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Deep clean complete!"

# Install the application to /usr/local/bin
install: $(TARGET)
	@echo "Installing to /usr/local/bin..."
	@sudo cp $(TARGET) /usr/local/bin/
	@echo "Installation complete!"

# Run with a test query
test: $(TARGET)
	@echo "Running test search..."
	@$(TARGET) "example search"

# Display help information
help:
	@echo "Terminal Search Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the application (default)"
	@echo "  clean      - Remove build files"
	@echo "  clean-all  - Remove build directories"
	@echo "  install    - Install to /usr/local/bin"
	@echo "  test       - Build and run a test search"
	@echo "  help       - Display this help information"

# Add .env file for API credentials if not exists
.env:
	@echo "# API Credentials" > .env
	@echo "GOOGLE_API_KEY=your_api_key_here" >> .env
	@echo "GOOGLE_CX=your_search_engine_id_here" >> .env
	@echo "Created .env file template. Update with your credentials."

# Create an example .env file
env-setup: .env
	@echo ".env file is ready. Remember to add it to .gitignore!"

# Ensure clean doesn't try to run if there's a file called 'clean'
.PHONY: all clean clean-all install test help directories env-setup