lib		:= libuthread.a
objc	:= uthread.o preempt.o queue.o context.o

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
	$(Q)$(RCC) $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objc) $(deps)