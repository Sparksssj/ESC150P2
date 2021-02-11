lib		:= libuthread.a
objc	:= uthread.o preempt.o queue.o context.o
inter	:= uthread.d preempt.d queue.d context.d
CC		:= gcc
RCS		:= ar rcs
CFLAGS	:= -Wall -Wextra -Werror -MMD

# CFLAGS += -g

ifneq ($(V), 1)
Q = @
endif

all: $(lib)

deps	:= $(patsubset %.o,%.d,$(objc))
-include $(deps)

$(lib): $(objc)
	@echo "RCS $@"
	$(Q)$(RCS) $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objc) $(deps) $(inter)