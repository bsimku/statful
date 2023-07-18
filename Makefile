TARGET = statbar

MKDIR ?= mkdir -p
O ?= .
CFLAGS ?=

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:%=$(O)/%.o)

$(O)/$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

$(O)/%.c.o: %.c
	$(MKDIR) $(dir $@)
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	$(RM) $(OBJECTS)
	$(RM) $(O)/$(TARGET)
