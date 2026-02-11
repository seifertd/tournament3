.PHONY: clean

CC ?= cc

ifeq "$(CC)" "clang"
	CFLAGS := -Wall -Wextra -Wno-string-concatenation -pedantic -std=c99 -O3 -lm
else
	CFLAGS := -Wall -Wextra -Wunused-variable -Wno-stringop-truncation -pedantic -std=c99 -O3 -lm
endif

programs=pool testpool benchmark scoredetail

all: $(programs)

%: %.c pool.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(programs)
