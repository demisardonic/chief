CC := gcc
SRCDIR := src
BUILDDIR := build
TARGET := bin/chief
INCLUDEDIR := include

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -Wall -Werror
LIB := 
INC := -I $(INCLUDEDIR)

$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)"
	@mkdir -p $(shell dirname "$(TARGET)")
	@$(CC) $^ -o $(TARGET) $(LIB)
	@mkdir -p log

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo "Compiling $<"
	@mkdir -p $(shell dirname "$@")
	@$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo Removing generated file
	@$(RM) -rf $(BUILDDIR) $(TARGET)
	@$(RM) $(shell find . -type f -name "*~")

rebuild: clean $(TARGET)

.PHONY: clean
