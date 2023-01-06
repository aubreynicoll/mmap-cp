P := mmcp
OBJECTS := $(P).o
CC := gcc
CFLAGS := -I$(HOME)/local/include -Wall -Wextra
LDFLAGS := -L$(HOME)/local/lib
LDLIBS :=

ifeq ($(BUILD_PROFILE), release)
	CFLAGS := $(CFLAGS) -Werror -O3
else
	CFLAGS := $(CFLAGS) -g -Og -pg
	LDFLAGS := $(LDFLAGS) -pg
endif

$(P): $(OBJECTS)

clean:
	rm -rf $(P) *.o
