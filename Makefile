TARGETS = randcp cookie

LDFLAGS = -lpthread
CFLAGS = -Wall
MANPAGE = randcp.1

DESTDIR ?= /usr/bin
MANDIR ?= /usr/man/man1

all: CFLAGS += -O1		#optimise, only in non-debug mode
all: $(TARGETS)

$(TARGETS):

# In debug mode, enable profiling too.
debug: CFLAGS += -ggdb -DDEBUG -pg
debug: $(TARGETS)

install: $(TARGETS) $(MANPAGE)
	install $(TARGETS) $(DESTDIR)
	install $(MANPAGE) $(MANDIR)

test:
	@echo "test sucess"

clean:
	rm -f *.o $(TARGETS)

.PHONY: clean test all
