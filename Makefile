# Makefile for Galactic Shmup
# Supports macOS with Homebrew raylib installation

# Compiler settings
CC = clang
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = galactic_shmup
SRCDIR = shmup
OBJDIR = build

# Detect raylib installation
RAYLIB_PATH = $(shell brew --prefix raylib 2>/dev/null)

# If raylib found via Homebrew
ifneq ($(RAYLIB_PATH),)
    INCLUDES = -I$(RAYLIB_PATH)/include
    LIBS = -L$(RAYLIB_PATH)/lib -lraylib
    FRAMEWORKS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
    # Fallback for manual installation
    INCLUDES = -I/usr/local/include -I/opt/homebrew/include
    LIBS = -L/usr/local/lib -L/opt/homebrew/lib -lraylib
    FRAMEWORKS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Default target
all: $(TARGET)

# Create build directory
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) $(FRAMEWORKS) -o $(TARGET)

# Install raylib via Homebrew
install-deps:
	@echo "Installing raylib via Homebrew..."
	brew install raylib

# Clean build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Run the game
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Print build info
info:
	@echo "Sources: $(SOURCES)"
	@echo "Objects: $(OBJECTS)"
	@echo "Raylib path: $(RAYLIB_PATH)"
	@echo "Includes: $(INCLUDES)"
	@echo "Libs: $(LIBS)"

# Force rebuild
rebuild: clean all

.PHONY: all clean run debug info install-deps rebuild
