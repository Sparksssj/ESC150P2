targets := libuthread.a
objc    := uthread.o preempt.o queue.o 

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -MMD

ifneq ($(V), 1)
Q = @
endif

ifneq ($(D), 1)
CFLAGS	+= -02
else 
CFLAGS	+= -g
endif

all: $(targets)

deps	:= $(patsubst %.o,%.d,$(objc))
-include $(deps)

libuthread.a: $(objc)
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm -f $(targets) $(objc) $(deps)