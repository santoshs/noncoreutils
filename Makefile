TARGET = randcp
SRC = randcp.c
OBJS = $(SRC:.c=.o)
LDFLAGS = -lpthread
CFLAGS = -Wall
MANPAGE = randcp.1

DESTDIR ?= /usr/bin
MANDIR ?= /usr/man/man1

all: CFLAGS += -O1		#optimise, only in non-debug mode
all: $(TARGET)

# In debug mode, enable profiling too.
debug: CFLAGS += -ggdb -DDEBUG -pg
debug: $(TARGET)

$(TARGET): $(OBJS)

install: $(TARGET) $(MANPAGE)
	install $(TARGET) $(DESTDIR)
	install $(MANPAGE) $(MANDIR)

test:
	@echo "test sucess"

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean test all
