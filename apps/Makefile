CC       = cc
CFLAGS   = -O -Wall
SRC      = countdown.c
PROGS    = countdown
FILTER   = -DEXT_FILTER

all: $(PROGS)

$(PROGS): $(SRC)
	$(CC) $(LDFLAGS) $(CFLAGS) $(FILTER) $(SRC) -o $@
