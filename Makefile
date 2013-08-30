TARGET = randcp
SRC = randcp.c
OBJS = $(SRC:.c=.o)
LDFLAGS = -lpthread
CFLAGS = -Wall

all: CFLAGS += -O1		#optimise, only in non-debug mode
all: $(TARGET)

# In debug mode, enable profiling too.
debug: CFLAGS += -ggdb -DDEBUG -pg
debug: $(TARGET)

$(TARGET): $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean all
